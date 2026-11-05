# Introduction

The [Parsing Expression Grammar Template Library](https://github.com/taocpp/PEGTL) (PEGTL) is a zero-dependency C++ header-only parser combinator library for creating parsers according to a [Parsing Expression Grammar](#parsing-expression-grammars) (PEG).


## Contents

* [Hello, World!](#hello-world)
* [Structure](#structure)
* [Namespaces](#namespaces)
* [Parsing Expression Grammars](#parsing-expression-grammars)
* [Definitions](#definitions)


## Hello, World!

Since the PEGTL is a parser library our "Hello, world!" example first parses, rather than prints, the string `Hello, foo!`, allowing for any sequence of alphabetic ASCII characters in place of `foo`.

```c++
#include <string>
#include <iostream>

// Include the commonly used parts of the core
// library with a single include directive:

#include <tao/pegtl.hpp>

// The library resides in the namespace TAO_PEGTL_NAMEPACE
// which by default is a macro defined to tao::pegtl. This
// can be changed in include/tao/pegtl/config.hpp.

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace hello
{
   // Parsing rule that matches a literal "Hello, ".

   struct prefix
      : pegtl::string< "Hello, " >
   {};

   // Parsing rule that matches a non-empty sequence of
   // alphabetic ascii-characters (with greedy-matching).
   // As PEG or Posix regex this would be '[a-zA-Z]+'.

   struct name
      : pegtl::plus< pegtl::alpha >
   {};

   // Parsing rule that matches a sequence of the 'prefix'
   // rule, the 'name' rule, a literal "!", and 'eof'
   // (end-of-file/input). The 'must' makes the parser
   // throw an exception when a sub-rule doesn't match
   // (instead of returning 'false' and possibly doing
   // some backtracking depending on the grammar).

   struct grammar
      : pegtl::must< prefix, name, pegtl::one< '!' >, pegtl::eof >
   {};

   // Class template for user-defined semantic actions;
   // the non-specialized default case does nothing.

   template< typename Rule >
   struct action
          : pegtl::nothing< Rule >
   {};

   // Specialisation of the user-defined action to do
   // something when the 'name' rule succeeds; is called
   // with the portion of the input that matched the rule,
   // packaged in an instance of pegtl::action_input<>.

   template<>
   struct action< name >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& v )
      {
         v = in.string();
      }
   };

}  // namespace hello

int main( int argc, char** argv )
{
   if( argc < 2 ) {
      return 1;
   }
   // Start a parsing run of argv[1] with the string
   // variable 'name' as additional argument that will
   // be passed to all called actions, including the
   // one we attached to the 'name' rule above.

   std::string name;
   pegtl::argv_input in( argv, 1 );

   if( !pegtl::parse< hello::grammar, hello::action >( in, name ) ) {
      std::cout << "I can't parse you!" << std::endl;
      return 1;
   }
   std::cout << "Good bye, " << name << "!" << std::endl;
   return 0;
}
```

Assuming the current directory is the main directory of the PEGTL this source can be found in `src/example/hello_world.cpp`.
On Linux and Unix, including macOS, this can be compiled with a command like

```sh
$ g++ --std=c++23 -Iinclude src/example/hello_world.cpp -o hello_world
```

or with a call to `make(1)` to build *all* examples and tests via the included `Makefile`.
Once the example is built it can be invoked as follows (taking care to use single quotes as shown, and noting that the executable will reside in `build/bin/example/hello_world` when using `make`):

```sh
$ ./hello_world 'Hello, world!'
Good bye, world!
$ ./hello_world 'Hello, Colin!'
Good bye, Colin!
$ ./hello_world 'Howdy, Paula!'
I can't parse you!
```


## Structure

The PEGTL contains both header and implementation files.
The actual library is header-only and requires no compilation.

### Header Files

The header files can be classified and grouped as follows.

 1. The core library headers that are included with `<tao/pegtl.hpp>`.
 2. The additional library headers in their respective sub-directories.
 2. The unofficial library headers in the `example` and `extra` sub-directories.
 3. The deprecated library headers in the `deprecated` sub-directory.

*The main and additional headers form the official public API of this library and are subject to [semantic versioning](https://semver.org/).*

The example and extra headers are considered too niche or experimental for inclusion in the official public API.
They are *not* subject to [semantic versioning](https://semver.org/) and can change at any time.

The deprecated headers are ones we expect to be rarely, if ever, used.
If you are using something deprecated please either copy it to your project and or let us know before we remove it.

| Directory | Contents |
| --------- | -------- |
| `include/tao/pegtl.hpp` | Core header "include all" |
| `include/tao/pegtl/action/` | [Additional actions](Action-Reference.md) |
| `include/tao/pegtl/binary/` | [Binary rules](Rule-Reference.md#binary) |
| `include/tao/pegtl/control/` | [Additional controls](Control-Reference.md) |
| `include/tao/pegtl/debug/` | [Debug facilities](Debug-Facilities.md) |
| `include/tao/pegtl/deprecated/` | [Deprecated headers](Extra-Reference.md#deprecated) |
| `include/tao/pegtl/example/` | [Example grammars](Example-Reference.md) |
| `include/tao/pegtl/extra/` | [Extra headers](Extra-Reference.md) |
| `include/tao/pegtl/stream/` | [Stream parsing](Stream-Parsing.md) |
| `include/tao/pegtl/unicode/` | [Unicode rules](Rule-Reference.md#unicode) |

The header files in any `internal/` sub-directory, and all C++ definitions and declarations in any `internal` sub-namespace, are *private* to the library.

### Implementation Files

There are two kinds of implementation files, tests and examples, found in `src/test/` and `src/example/`, respectively.
Neither is considered part of the public API wherefore neither is subject to semantic versioning.
The examples are listed in the [Example Reference](Example-Reference.md).


## Namespaces

By default, the entire PEGTL resides in namespace `tao::pegtl`.
This can be changed in `include/tao/pegtl/config.hpp` as explained in [Embedding in Libraries](Install-Guide.md#embedding-in-libraries).
**The entire PEGTL documentation assumes the default namespace.**

Some parts of the library use sub-namespaces, for example the parsing rules specific to UTF-8 encoded text are in namespace `tao::pegtl::utf8`.
Similarly the parsing rules for ASCII text are in namespace `tao::pegtl::ascii`, which is an [inline namespace](https://en.cppreference.com/w/cpp/language/namespace.html), making them (also) accessible "as if" they were in namespace `tao::pegtl`.

For this reason we frequently put the `ascii` in brackets and write `tao::pegtl::(ascii::)` to designate the ASCII namespace throughout this documentation.

Everything that is considered private to the library resides in an `internal` namespace.
Private parts of the library are not considered part of the library interface and are therefore not subject to semantic versioning.

Macros do not respect namespaces and have a `TAO_PEGTL_` prefix instead.


## Parsing Expression Grammars

The PEGTL creates parsers according to a [Parsing Expression Grammar](http://en.wikipedia.org/wiki/Parsing_expression_grammar) (PEG).
The table below shows how the classical PEG combinators, or composite parsing expressions, map to PEGTL [rule class templates](Rules-and-Grammars.md).

| PEG                               | `tao::pegtl::`                                                                                            |
| ---                               | ---                                                                                                       |
| &*e*                              | [`at< R... >`](Rule-Reference.md#at-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>         |
| !*e*                              | [`not_at< R... >`](Rule-Reference.md#not_at-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup> |
| *e*?                              | [`opt< R... >`](Rule-Reference.md#opt-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>       |
| *e*+                              | [`plus< R... >`](Rule-Reference.md#plus-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>     |
| *e*<sub>1</sub>*e*<sub>2</sub>    | [`seq< R... >`](Rule-Reference.md#seq-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>       |
| *e*<sub>1</sub> / *e*<sub>2</sub> | [`sor< R... >`](Rule-Reference.md#sor-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>       |
| *e**                              | [`star< R... >`](Rule-Reference.md#star-r-) <sup>[(combinators)](Rule-Reference.md#combinators)</sup>     |

And the next table shows how some of the more common atomic PEG expressions are expressed with PEGTL [rule classes](Rules-and-Grammars.md).

| PEG                               | `tao::pegtl::`                                                                         |
| ---                               | ---                                                                                    |
| *E*                               | [`eof`](Rule-Reference.md#eof) <sup>[(atomic)](Rule-Reference.md#atomic)</sup>         |
| ε                                 | [`success`](Rule-Reference.md#success) <sup>[(atomic)](Rule-Reference.md#atomic)</sup> |
| ⊥                                 | [`failure`](Rule-Reference.md#failure) <sup>[(atomic)](Rule-Reference.md#atomic)</sup> |
| .                                 | [`any`](Rule-Reference.md#any) <sup>[(ascii)](Rule-Reference.md#ascii)</sup>           |
| "a"                               | [`one< 'a' >`](Rule-Reference.md#one-c-) <sup>[(ascii)](Rule-Reference.md#ascii)</sup> |
| "[a-h]"                           | [`range< 'a', 'h' >`](Rule-Reference.md#range-c-d-) <sup>[(ascii)](Rule-Reference.md#ascii)</sup> |
| "[a-zA-Z]"                        | [`alpha`](Rule-Reference.md#alpha) <sup>[(ascii)](Rule-Reference.md#ascii)</sup>       |

The PEGTL comes with [dozens of rules](Rule-Reference.md#index) for convenience and advanced features, and the possibility to [implement custom rules](Rules-and-Grammars.md#implementing-rules).


## Definitions

* A (parsing) [**rule**](Rules-and-Grammars.md) is a class that models a [(production) rule](https://en.wikipedia.org/wiki/Production_(computer_science)) of a [formal grammar](https://en.wikipedia.org/wiki/Formal_grammar), or a [parser combinator](https://en.wikipedia.org/wiki/Parser_combinator).
* A [**grammar**](Rules-and-Grammars.md) is a set of one or more related (parsing) rules, with one (or more) designated top-level rules as entry-point(s).
* **Input data** is a sequence of objects - often of type `char` - that is intended to be parsed.
* An [**input**](Inputs-and-Parsing.md) is a class that adheres to an informal input interface and represents some input data.
* A (semantic) [**action**](Actions-and-States.md) is a class with a static [`apply()`](Actions-and-States.md#apply) or [`apply0()`](Actions-and-States.md#apply0) function -- and/or, for advanced use cases, a static [`match()`](Actions-and-States.md#match) function.
* A [**control**](Control-and-Normal.md) is a class that adheres to an informal control interface and is in control of important behind-the-scenes details of a parsing run.
* The [**states**](Actions-and-States.md#states) are user-defined objects that are passed to all rules, actions and control functions.
* A [**parsing run**](Inputs-and-Parsing.md) is everything that happens during a call to `tao::pegtl::parse()` with a grammar and, optionally, an action, a control and states.
* A [**nested parsing**](Inputs-and-Parsing.md#nested-parsing) run similarly refers to a call to `tao::pegtl::parse_nested()` during a parsing run (usually from an action).
* A [**position**](Inputs-and-Parsing.md#position-classes) is an instance of a class that indicates an object in the input data, possibly with auxiliary information like filename and line number.
* Input is **consumed** when the reference to what is considered the current object in the input data is advanced.
* [**Stream parsing**](Stream-Parsing.md) refers to parsing with an input where only a small portion of input data is kept in a contiguous memory buffer.
* An action `A` is **attached** to a rule `R` when the specialization `A< R >` has an `apply()`, `apply0()` or `match()` function.
* An action is **applied** when its `apply()` or `apply0()` function is called after the rule it is attached to succeeded.
* **Success** is when a [`match()` function](Rules-and-Grammars.md#match-function) returns `true`.
* **Local failure** is when a [`match()` function](Rules-and-Grammars.md#match-function) returns `false` (which can lead to backtracking).
* **Global failure** is when a [`match()` function](Rules-and-Grammars.md#match-function) throws an exception (which usually aborts the parsing run).
* The **matched input** is the portion of the input data consumed by a parsing rule during a successful call to its `match()` function.
* The [**grammar analysis**](Debug-Facilities.md#grammar-analysis) is an algorithm that checks a grammar for the occurrence of potential infinite loops stemming e.g. from [left recursion](https://en.wikipedia.org/wiki/Left_recursion).


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
