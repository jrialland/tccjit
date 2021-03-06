#include "tccjit.hpp"

#include <cstring>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>

#include <sys/mman.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
/**
 * utility method, check if the given file exists
 */
static bool is_file(const std::string &name) {
	std::ifstream f(name.c_str());
	return f.good();
}

////////////////////////////////////////////////////////////////////////////////
/**
 * utility method : write len bytes located at 'data' into the dest file.
 * raise a std::runtime_error if there is a problem
 */
static void write_file(uint8_t* data, unsigned len, const char * dest) {
	FILE *f = fopen(dest, "wb");
	if(f == nullptr) {
		throw std::runtime_error(strerror(errno));
	}
	unsigned done = 0;
	while(done < len) {
		unsigned thisround = std::min(1024u, len-done);
		int written = fwrite(data + done, sizeof(uint8_t), thisround, f);
		if(written != thisround) {
			throw std::runtime_error(strerror(errno));
		}
		done += written;
	}
	fclose(f);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * libtcc1.a file contents (generated by cmake)
 */
extern uint8_t libtcc1_a_data[];
extern unsigned libtcc1_a_size;

////////////////////////////////////////////////////////////////////////////////
/**
 * tries to find where libtcc1.a is. returns the name of the directory that might contain it
 */
static std::string check_libtcc1a() {

	std::string path = "/usr/lib/x86_64-linux-gnu/tcc/libtcc1.a";
	if(is_file(path)) {
		return "/usr/lib/x86_64-linux-gnu/tcc";
	}

	path = "/usr/local/lib/tcc/libtcc1.a";
	if(is_file(path)) {
		return "/usr/local/lib/tcc";
	}
	
	path = "./libtcc1.a";
	if(!is_file(path)) {
		try {
			write_file(libtcc1_a_data, libtcc1_a_size, path.c_str());
		} catch(std::runtime_error& err) {
			fprintf(stderr, "%s\n", err.what());
		}
	}
	return ".";
}

////////////////////////////////////////////////////////////////////////////////
#ifdef __linux 
#include <libgen.h>
#include <link.h>
extern "C" {
	// scan loaded dynamic libraries, for each one , add its direcory to the tcc context's library path
	static int dl_iterate_cb(struct dl_phdr_info *info, size_t size, void *data) {
		char buf[1024];
		strncpy(buf, info->dlpi_name, 1024);
		tcc_add_library_path(reinterpret_cast<TCCState*>(data), dirname(buf));
		return 0;
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
void addErrorMsg(void *v, const char *msg) {
	((std::vector<string>*)v)->push_back(msg);
}

////////////////////////////////////////////////////////////////////////////////
jit::Module::Module(const std::string &code, bool autolink) : mem(nullptr), mem_size(0) {
	
	// create tcc context
	tcc = tcc_new();

	if(tcc == nullptr) {
		throw std::runtime_error("tcc context initialization failed");
		return;
	}

	std::vector<string> errorMsgs;
	tcc_set_error_func(tcc, &errorMsgs, addErrorMsg);

	// look for a libtcc1.a file in various locations
	std::string lib_path = check_libtcc1a();

	// add it to the path (required)
	tcc_set_lib_path(tcc, lib_path.c_str());

	// in-memory generation
	tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
	
#ifdef __linux
	// add needed library (c runtime, etc)
	dl_iterate_phdr(dl_iterate_cb, tcc);
#endif

	// compile input string
	if(tcc_compile_string(tcc, code.c_str()) != 0) {
		std::ostringstream ss;
		ss << "compilation failed : \n";
		for(auto s : errorMsgs) {
			ss << s << endl;
		}
		throw std::runtime_error(ss.str());
	}
	
	// link
	if(autolink) {
		link();
	}
}

////////////////////////////////////////////////////////////////////////////////
void jit::Module::link() {
	std::vector<string> errorMsgs;
	tcc_set_error_func(tcc, &errorMsgs, addErrorMsg);
	mem_size = tcc_relocate(tcc, nullptr);
	if(mem_size == -1) {
		std::ostringstream ss;
		ss << "relocation failed : \n";
		for(auto s : errorMsgs) {
			ss << s << endl;
		}
		throw std::runtime_error(ss.str());
	}
	mem = static_cast<char*>(malloc(sizeof(char) * mem_size));
	tcc_relocate(tcc, mem);
}

////////////////////////////////////////////////////////////////////////////////
void* jit::Module::get_symbol(const std::string &symbol) {
	if(mem == nullptr) {
		link();
	}
	void *sym = tcc_get_symbol(tcc, symbol.c_str());
	if(sym == nullptr) {
		throw std::runtime_error("symbol not found : '" + symbol +"'");
	}
	return sym;
}

////////////////////////////////////////////////////////////////////////////////
void jit::Module::add_symbol(const std::string& name, void *sym) {
	if(mem != nullptr) {
		throw std::runtime_error("module already linked");
	}
	tcc_add_symbol(tcc, name.c_str(), sym);
}

////////////////////////////////////////////////////////////////////////////////
jit::Module::~Module() {
	tcc_delete(tcc);
	if(mem != nullptr) {
		free(mem);
	}
}
