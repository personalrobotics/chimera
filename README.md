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
- yaml-cpp

**On Ubuntu**

```
sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.6 main'
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get install llvm-3.6-dev llvm-3.6-tools libclang-3.6-dev libedit-dev libyaml-cpp-dev
```

**On Mac OS X** Build with:

```bash
brew install yaml-cpp --with-static-lib
PKG_CONFIG_PATH=/usr/local/Cellar/yaml-cpp/0.5.2/lib/pkgconfig cmake -DLLVM_DIR=/usr/local/opt/llvm/share/llvm/cmake ..
```

## Usage ##
Let's try running chimera on itself!

```
$ cd [PATH TO CHIMERA]
$ rm -rf build && mkdir -p build && cd build
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
$ make
$ chimera -p . -o chimera_py_binding.cpp ../src/chimera.cpp
```

## Troubleshooting ##

- **Cannot find "stdarg.h" when running `chimera`.**
  https://bugs.launchpad.net/ubuntu/+source/llvm-defaults/+bug/1242300

  ```bash
  $ chimera -extra-arg "-I/usr/lib/clang/3.6/include" -p . -o chimera_py_binding.cpp ../src/chimera.cpp
  ```

[1]: http://www.oxforddictionaries.com/us/media/american_english/us_pron_ogg/c/chi/chime/chimera__us_1_rr.ogg
[2]: https://upload.wikimedia.org/wikipedia/commons/7/74/Speaker_icon.svg
