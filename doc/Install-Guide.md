# Install Guide

How to install the PEGTL, and verify and ensure that the system requirements are met.


## Contents

* [Requirements](#requirements)
* [Disabling Exceptions](#disabling-exceptions)
* [Disabling RTTI](#disabling-rtti)
* [Installation Packages](#installation-packages)
* [Using Vcpkg](#using-vcpkg)
* [Using Conan](#using-conan)
* [Using CMake](#using-cmake)
  * [CMake Installation](#cmake-installation)
  * [`find_package`](#find_package)
  * [`add_subdirectory`](#add_subdirectory)
  * [Mixing `find_package` and `add_subdirectory`](#mixing-find_package-and-add_subdirectory)
* [Distributions](#distributions)
* [Manual Installation](#manual-installation)
* [Embedding the PEGTL](#embedding-the-pegtl)
  * [Embedding in Binaries](#embedding-in-binaries)
  * [Embedding in Libraries](#embedding-in-libraries)
  * [Embedding in Library Interfaces](#embedding-in-library-interfaces)


## Requirements

The PEGTL requires a compiler that supports C++23 like

* GCC 15 or later with `--std=c++23`
* Clang 18 or later with `--std=c++23`
* VS 2026 MSVC 14.52 or later with `/std:c++23`

on the mainstream operating systems

* Linux
* macOS
* Windows
* Android

The PEGTL is written with an emphasis on clean code and compiles with `-pedantic`, `-Wall`, `-Wextra` and `-Werror`.
Newer compilers and newer versions of the C++ standard are fully supported.

The PEGTL is built with many small functions and relies greatly on the compiler for performance optimizations.
It is recommended to *always* compile with *some* optimizations enabled, especially function inlining.

Larger projects will frequently require the `/bigobj` option when compiling with Visual Studio on Windows.

The PEGTL should also be compatible with other C++23 capable compilers and work on other Unix or Unix-like operating systems.


## Disabling Exceptions

The PEGTL *is* compatible with `-fno-exceptions`, however some features and functions are not available or change their behavior without exceptions.

Parsing rules that throw or catch exceptions and therefore require exception support:

 * `if_must<>`
 * `if_must_else<>`
 * `list_must<>`
 * `must<>`
 * `opt_must<>`
 * `raise<>`
 * `raise_message< Text >`
 * `raise_message_chars< C... >`
 * `star_must<>`
 * `try_catch_any_raise_nested<>`
 * `try_catch_any_return_false<>`
 * `try_catch_raise_nested<>`
 * `try_catch_return_false<>`
 * `try_catch_std_raise_nested<>`
 * `try_catch_std_return_false<>`
 * `try_catch_type_raise_nested<>`
 * `try_catch_type_return_false<>`

Grammars and other classes that indirectly rely on exception support or are intrinsically linked to exceptions:

 * `tao/pegtl/parse_error.hpp`
 * `tao/pegtl/parse_error_base.hpp`
 * `tao/pegtl/control/must_if.hpp`
 * `tao/pegtl/example/http.hpp`.
 * `tao/pegtl/example/uri.hpp`.
 * `tao/pegtl/deprecated/integer.hpp`.
 * `tao/pegtl/extra/charconv.hpp`.

Facilities that use `std::perror()` and `std::terminate()` instead of `throw` when exceptions are disabled:

 * All I/O errors during file mapping and reading.
 * Some [extra](Extra-Reference.md) actions.

Some tests and examples are (partially) disabled when compiling without exception support.


## Disabling RTTI

The PEGTL is compatible with `-fno-rtti` on GCC, Clang, and MSVC.
GCC 16.1 requires RTTI due to [bug #91155](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91155).

On other compilers [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information) is required by default (for demangling, see `include/tao/pegtl/demangle.hpp`).
Let us know if you use such a compiler since an RTTI-free compiler-specific demangling function might be possible.


## Installation Packages

<a href="https://repology.org/metapackage/pegtl"><img align="right" hspace="20" src="https://repology.org/badge/vertical-allrepos/pegtl.svg" alt="Packaging status"></a>

Installation packages are available from several package managers.
Note that some of the listed packages are not updated regularly.


## Using Vcpkg

You can download and install the PEGTL using the [Vcpkg] package manager:

```bash
vcpkg install pegtl:x64-linux pegtl:x64-osx pegtl:x64-windows
```

The `pegtl` package in Vcpkg is kept up to date by the Vcpkg team members
and community contributors. If the version is out-of-date, please
[create an issue or pull request](https://github.com/Microsoft/vcpkg)
on the Vcpkg repository.

For more options and ways to use Vcpkg, please refer to the [Vcpkg documentation].


## Using Conan

You can download and install the PEGTL using the [Conan] package manager:

```bash
conan install taocpp-pegtl/<version>@
```

where `<version>` is the version of the PEGTL you want to use.

The `taocpp-pegtl` package in Conan is kept up to date by Conan team members
and community contributors. If the version is out-of-date, please
[create an issue or pull request](https://github.com/conan-io/conan-center-index)
on the Conan Center Index repository.

For more options and ways to use Conan, please refer to the [Conan documentation].


## Using CMake

### CMake Installation

The PEGTL can be built and installed using [CMake], e.g.

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make install
```

The above will install the PEGTL into the standard installation path on a UNIX
system, e.g. `/usr/local/include/`. To change the installation path, use:

```sh
$ cmake .. -DCMAKE_INSTALL_PREFIX=../install
```

in the above.

### `find_package`

Installation creates a `pegtl-config.cmake` which allows CMake
projects to find the PEGTL using `find_package`:

```cmake
find_package(pegtl)
```

This exports the `taocpp::pegtl` target which can be linked against any other
target. Linking against `taocpp::pegtl` automatically sets the include
directories and required flags for C++23 or later. For example:

```cmake
add_executable(myexe mysources...)
target_link_libraries(myexe PRIVATE taocpp::pegtl)
```

### `add_subdirectory`

The PEGTL can also be added as a dependency with `add_subdirectory`:

```cmake
add_subdirectory(path/to/PEGTL)
```

This also exports the `taocpp::pegtl` target which can be linked against any
other target just as with the installation case.

Due to the global nature of CMake targets the target `pegtl` is also defined,
but only `taocpp::pegtl` should be used for consistency.

If `PEGTL_BUILD_TESTS` is true then the test targets, `pegtl-test-*`, are also
defined and their corresponding tests registered with `add_test`.
If `PEGTL_BUILD_EXAMPLES` is true then the example targets, `pegtl-example-*`,
are also defined.

### Mixing `find_package` and `add_subdirectory`

With the advent of improved methods of managing dependencies (such as [Conan],
[CMake FetchContent]), multiple package inclusion methods need to be able to
co-exist.

If PEGTL was first included with `find_package` then subsequent calls to
`add_subdirectory(path/to/PEGTL)` will skip over the body of the
`CMakeLists.txt` and use the installed package if the version matches.
If the version does not match a fatal error will be signaled.

If PEGTL was first included with `add_subdirectory` then a dummy
`pegtl-config.cmake` is created and `pegtl_DIR` is set. Subsequent calls to
`find_package(pegtl)` will then use the already added package if the version
matches. If the version does not match a fatal error will be signaled.

Since CMake targets are global, there exists no way for a CMake project to use
two different versions of PEGTL simultaneously and signaling a fatal error
becomes the only practical way of handling the inclusion of multiple different
PEGTL versions.

For more options and ways to use CMake, please refer to the [CMake documentation].


## Distributions

Some [Linux distributions](https://distrowatch.org/), and projects like [Homebrew](https://brew.sh/) and [MacPorts](https://macports.org/), have PEGTL packages that can be installed with their respective native package manager.
Please check on the project homepage or with the package manager whether a PEGTL package is available.

We greatly appreciate the work of the people providing and maintaining these packages.


## Manual Installation

Since the PEGTL is a header-only library, _it doesn't itself need to be compiled_.
In terms of installation for use in other projects, the following steps are required.

- The `include/` directory and the `LICENSE_1_0.txt` file should be copied somewhere, e.g.

  - to `/usr/local/include/` in order to use it system-wide, or
  - to some appropriate directory within your project.

- A compatible compiler with appropriate compiler switches must be used.
- The compiler search-path for include files must include (no pun intended)
  the directory that contains the `tao/pegtl/` directory and `tao/pegtl.hpp` header.

The `Makefile` and `.cpp`-files included in the PEGTL distribution archive serve
as practical examples on how to develop grammars and applications with the PEGTL.
Invoking `make` in the main PEGTL directory builds all included example programs,
and builds and runs all unit tests.

The `Makefile` is as simple as possible, but should manage to build the examples
and unit tests on Linux with GCC and on macOS with Clang (as supplied by Apple).
When running into problems using other combinations, please consult the `Makefile`
for customizing the build process.


## Embedding the PEGTL

When embedding the PEGTL in other projects, several problems might show up
due to the nature of C++ header-only libraries. Depending on the scenario,
there are various ways of working around these problems.

### Embedding in Binaries

When creating application binaries, i.e. executable files, the PEGTL source
tree can be copied to some subdirectory in the application source, and added
to the compiler's or project's include paths. No further changes are needed.

### Embedding in Libraries

When writing libraries with the PEGTL, it has to be ensured that applications
that are built with these libraries, and that themselves use the PEGTL, do not
violate the One Definition Rule (ODR) as would be the case when application
and libraries contain different versions of the PEGTL.

Since the PEGTL does **not** guarantee ABI compatibility, not even across minor
or patch releases, libraries **have** to ensure that symbols for their embedded
PEGTL differ from those of the applications that also use their own PEGTL.

This can be achieved by changing the macro `TAO_PEGTL_NAMESPACE` which, by
default, is set to `tao::pegtl`. To change the namespace, simply define
`TAO_PEGTL_NAMESPACE` to a unique name before including the PEGTL, for example:

```c++
#define TAO_PEGTL_NAMESPACE mylib::pegtl

#include <tao/pegtl.hpp>
#include <tao/pegtl/example/json.hpp>

int main( int argc, char* argv[] )
{
   if( argc > 1 ) {
     mylib::pegtl::argv_input in( argv, 1 );
     mylib::pegtl::parse< mylib::pegtl::json::text >( in );
   }
}

```

### Embedding in Library Interfaces

When PEGTL headers are included in headers of a library, setting the namespace
to a unique name via `TAO_PEGTL_NAMESPACE` is not sufficient since both the
application's and the library's copy of the PEGTL use the same macro names.

In this case it is necessary to change the prefix of all macros of the embedded
PEGTL from `TAO_PEGTL_` to another unique string in order to prevent macros
from clashing. In a Unix-shell, the following command will achieve this:

```sh
$ sed -i 's/TAO_PEGTL_/MYLIB_PEGTL_/g' $(find -name '[^.]*.[hc]pp')
```

The above command needs to run from the top-level directory of the embedded
PEGTL. Additionally, `MYLIB_PEGTL_NAMESPACE` needs to be set as explained
above; alternatively, since the PEGTL source is already being modified,
`include/tao/pegtl/config.hpp` can be changed to use the desired namespace.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt

[Vcpkg]: https://github.com/Microsoft/vcpkg
[Vcpkg documentation]: https://github.com/Microsoft/vcpkg/tree/master/docs/index.md
[Conan]: https://conan.io/
[Conan documentation]: https://docs.conan.io/
[CMake]: https://cmake.org/
[CMake documentation]: https://cmake.org/documentation/
[CMake FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html
