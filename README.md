# chimera #

> ##### chi·me·ra #####
> ##### /kīˈmirə,kəˈmirə/ [![speaker][2]][1] #####
> _*informal*, noun_
>
> 1. a thing that is hoped or wished for but in fact is illusory or impossible to achieve.
> 2. a utility to generate Boost.Python bindings for C++ code.

Chimera is a tool for generating Boost.Python bindings from C/C++ header files.
It uses the Clang/LLVM toolchain, making it capable of automatically handling
fairly complex source files.

## Usage ##

```bash
$ ./chimera -c <yaml_config_file> -o <output_path> my_cpp_header1.h my_cpp_header2.h -- [compiler args]
```

## Installation ##

** Requirements **

- libclang 3.6
- llvm 3.6 (+ tools)
- libedit
- yaml-cpp

**On Ubuntu**

```bash
sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.6 main'
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get install llvm-3.6-dev llvm-3.6-tools libclang-3.6-dev libedit-dev libyaml-cpp-dev
```

**On Mac OS X** Build with:

```bash
brew install yaml-cpp --with-static-lib
PKG_CONFIG_PATH=/usr/local/Cellar/yaml-cpp/0.5.2/lib/pkgconfig cmake -DLLVM_DIR=/usr/local/opt/llvm/share/llvm/cmake ..
```

## Example ##
Let's try running chimera on itself!

```bash
$ cd [PATH TO CHIMERA]
$ rm -rf build && mkdir -p build && cd build
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
$ make
$ chimera -p . -o chimera_py_binding.cpp ../src/chimera.cpp
```

## Configuration ##

```yaml
# The C++ namespaces that will be extracted by Chimera
namespaces:
  - dart::dynamics
  - dart::math

# Selected types that should have special handling.
# (Not implemented yet.)
types:
  'class BodyNode':
    convert_to: 'class BodyNodePtr'

# Selected function and class declarations that need custom parameters.
declarations:
  'const Eigen::Vector4d & ::dart::dynamics::Shape::getRGBA() const':
    return_value_policy: ::boost::python::copy_const_reference
  'bool ::dart::dynamics::Skeleton::isImpulseApplied() const':
    source: 'test.cpp.in'
  'const Eigen::Vector3d & ::dart::dynamics::Shape::getBoundingBoxDim() const':
    content: '/* Instead of implementing this function, insert this comment! */'
  'Eigen::VectorXd & ::dart::optimizer::GradientDescentSolver::getEqConstraintWeights()': ~
    # This declaration will be suppressed.
  '::dart::dynamics::Shape':
    name: Shape
    bases: []
    noncopyable: true
```

## Troubleshooting ##

- **Cannot find "stdarg.h" when running `chimera`.**
  https://bugs.launchpad.net/ubuntu/+source/llvm-defaults/+bug/1242300

  ```bash
  $ chimera -extra-arg "-I/usr/lib/clang/3.6/include" -p . -o chimera_py_binding.cpp ../src/chimera.cpp
  ```

[1]: http://www.oxforddictionaries.com/us/media/american_english/us_pron_ogg/c/chi/chime/chimera__us_1_rr.ogg
[2]: https://upload.wikimedia.org/wikipedia/commons/7/74/Speaker_icon.svg
