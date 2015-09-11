# chimera #

> ##### chi·me·ra #####
> ##### /kīˈmirə,kəˈmirə/ [![speaker][2]][1] #####
> _*informal*, noun_
>
> 1. a thing that is hoped or wished for but in fact is illusory or impossible to achieve.
> 2. a utility to generate Boost.Python bindings for C++ code.

## Requirements ##

- libclang3.6-dev
- libllvm3.6-dev

**On Ubuntu**
```
sudo apt-get install libclang3.6-dev llvm-3.6-dev
```

## Usage ##
```
$ chimera ${CXX_FLAGS} \
    --input my_binding.h --output my_binding.cpp
```

[1]: http://www.oxforddictionaries.com/us/media/american_english/us_pron_ogg/c/chi/chime/chimera__us_1_rr.ogg
[2]: https://upload.wikimedia.org/wikipedia/commons/7/74/Speaker_icon.svg
