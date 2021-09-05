#include "tccjit.hpp"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iostream>
using namespace std;
namespace fs = std::filesystem;

bool is_file(const std::string &name) {
	std::ifstream f(name.c_str());
	return f.good();
}

extern uint8_t libtcc1_a_data[];
extern unsigned libtcc1_a_size;

void write_file(uint8_t* data, unsigned len, const char * dest) {
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

void ensure_libtcc1a() {
	std::string path = "./libtcc1.a";
	if(!is_file(path)) {
		write_file(libtcc1_a_data, libtcc1_a_size, path.c_str());
	}
}

#ifdef __linux 
#include <libgen.h>
#include <link.h>
extern "C" {
	static int dl_iterate_cb(struct dl_phdr_info *info, size_t size, void *data) {
		char buf[1024];
		strncpy(buf, info->dlpi_name, 1024);
		tcc_add_library_path(reinterpret_cast<TCCState*>(data), dirname(buf));
		return 0;
	}
}
#endif

jit::Module::Module(const std::string &code) {
	
	ensure_libtcc1a();

	tcc = std::shared_ptr<TCCState>(tcc_new(), tcc_delete);
	
	if(tcc.get() == nullptr) {
		throw std::runtime_error("tcc context initialization failed");
		return;
	}

	tcc_set_lib_path(tcc.get(), ".");

	
	tcc_set_output_type(tcc.get(), TCC_OUTPUT_MEMORY);

#ifdef __linux
	dl_iterate_phdr(dl_iterate_cb, tcc.get());
#endif

	if(tcc_compile_string(tcc.get(), code.c_str()) != 0) {
		throw std::runtime_error("compilation failed");
	}

	int size = tcc_relocate(tcc.get(), NULL);
	if(size == -1) {
		throw std::runtime_error("relocation failed");
	}

	mem.reset(new char[size]);
	tcc_relocate(tcc.get(), mem.get());
}

void* jit::Module::get_symbol(const std::string &symbol) const {
	return tcc_get_symbol(tcc.get(), symbol.c_str());
}

jit::Module::Module(const jit::Module& module) : tcc(module.tcc), mem(module.mem) {
}

jit::Module::~Module() {
}

