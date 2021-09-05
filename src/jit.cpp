#include "jit.hpp"

#include <libtcc.h>

#include <libgen.h>

#include <cstring>
#include <stdexcept>

#ifdef __linux 
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
	
	tcc = std::shared_ptr<TCCState>(tcc_new(), tcc_delete);
	
	if(tcc.get() == nullptr) {
		throw std::runtime_error("tcc context initialization failed");
		return;
	}
	
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

