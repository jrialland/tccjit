#ifndef jit_hpp
#define jit_hpp
#pragma once

#include <libtcc.h>
#include <string>
#include <functional>
#include <memory>

namespace jit {

class Module {

private:

	std::shared_ptr<TCCState> tcc;

	std::shared_ptr<char[]> mem;

	void *get_symbol(const std::string &symbol);

	bool linked;
	
	virtual void link();

public:

	Module(const std::string &code, bool autolink=false);

	Module(const Module& module);

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
