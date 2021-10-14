#ifndef jit_hpp
#define jit_hpp
#pragma once

#include <libtcc.h>
#include <string>
#include <functional>

namespace jit {

class Module {

private:

	TCCState *tcc;

	char *mem;

	size_t mem_size;

	void *get_symbol(const std::string &symbol);
	
	virtual void link();

	Module(const Module &) = delete;

public:

	Module(const std::string &code, bool autolink=false);

	virtual ~Module();

	virtual void add_symbol(const std::string& name, void *sym);

	template<typename Fn> std::function<Fn> fn(const std::string& symbol) {
		return reinterpret_cast<Fn*>(get_symbol(symbol));
	}

	template<typename T> T* sym(const std::string& symbol) {
		return reinterpret_cast<T*>(get_symbol(symbol));
	}

};

} // namespace tcc

#endif // #ifndef jit_hpp
