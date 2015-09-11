# chimera #

> ##### chi·me·ra #####
> ##### /kīˈmirə,kəˈmirə/ [![speaker][2]][1] #####
> _*informal*, noun_
>
> 1. a thing that is hoped or wished for but in fact is illusory or impossible to achieve.
> 2. a utility to generate Boost.Python bindings for C++ code.

## Requirements ##

- libclang 3.6
- llvm 3.6 (+ tools)
- libedit

**On Ubuntu**
```
sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.6 main'
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get install llvm-3.6-dev llvm-3.6-tools libclang-3.6-dev libedit-dev
```

## Usage ##
Let's try running chimera on itself!
```
$ cd [PATH TO CHIMERA]
$ rm -rf build && mkdir -p build && cd build
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
$ make
$ chimera -p . --output chimera_py_binding.cpp ../src/chimera.cpp
```

[1]: http://www.oxforddictionaries.com/us/media/american_english/us_pron_ogg/c/chi/chime/chimera__us_1_rr.ogg
[2]: https://upload.wikimedia.org/wikipedia/commons/7/74/Speaker_icon.svg
