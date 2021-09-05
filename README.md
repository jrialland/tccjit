
A small C++ wrapper around libtcc (❤️) (https://bellard.org/tcc/)

Compiles code at runtime :

```c++
#include "tccjit.hpp"

int main() {

    // generate the code
    jit::Module module(
        "void func(char * txt) {"
        "    printf(\"Hello, %s\\n\", txt);"
        "}"
    );

    // fetch the function
    auto func = module.fn<void(char*)>("func");

    // run it
    func("World);

    return 0;
}
```



cmake generates a dynamic library that embeds libtcc, so it can be used directly in your c++ project :

```
g++ -L. -ltccjit example.cpp -o example
```