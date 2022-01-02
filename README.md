# utu

`utu` uses the [loris](http://www.cerlsoundgroup.org/Loris/) library to provide
sound analysis, synthesis, and morphing functionality. `utu` defines an
alternative JSON based file format for partial data to allow the analysis output
to be used in a wide number of environments.

## building

`utu` requires a C++17 capable compiler, tested compilers include:

| compiler | version | platform |
| -------- | ------- | -------- |
| clang | Apple clang version 13.0.0 | arm64-apple-darwin21.2.0 |
| gcc | 10.2.1 20210110 (Debian 10.2.1-6) | aarch64-linux-gnu |

### dependencies

`utu` builds and statically links several third party libraries from source
which requires installation of the GNU autotools suite.

on linux:

```
sudo apt install cmake autotools-dev automake autoconf m4 libtool libasound2-dev
```

on macos:

```
brew install cmake ninja automake autoconf m4 libtool
```

### building

once dependencies are installed building `utu` itself can be done as follows:

```
git clone ssh://git@github.com/ngwese/utu.git
cd utu
git submodule update --init --depth 1
mkdir build
cd build
cmake ..
cmake --build .
./bin/Debug/Debug/utu --help
```
