# Welcome to the PEGTL

[![Windows](https://github.com/taocpp/PEGTL/actions/workflows/windows.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/windows.yml)
[![macOS](https://github.com/taocpp/PEGTL/actions/workflows/macos.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/macos.yml)
[![Linux](https://github.com/taocpp/PEGTL/actions/workflows/linux.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/linux.yml)
[![Android](https://github.com/taocpp/PEGTL/actions/workflows/android.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/android.yml)
<br>
[![clang-analyze](https://github.com/taocpp/PEGTL/actions/workflows/clang-analyze.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/clang-analyze.yml)
[![clang-tidy](https://github.com/taocpp/PEGTL/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/clang-tidy.yml)
[![Sanitizer](https://github.com/taocpp/PEGTL/actions/workflows/sanitizer.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/sanitizer.yml)
[![CodeQL](https://github.com/taocpp/PEGTL/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/taocpp/PEGTL/actions/workflows/codeql-analysis.yml)
[![Codecov](https://codecov.io/gh/taocpp/PEGTL/branch/main/graph/badge.svg?token=ykWa8RRdyk)](https://codecov.io/gh/taocpp/PEGTL)

The **Parsing Expression Grammar Template Library** (PEGTL) is a zero-dependency [C++](https://en.cppreference.com/w/cpp.html) [header-only](https://en.wikipedia.org/wiki/Header-only) [parser combinator](https://en.wikipedia.org/wiki/Parser_combinator) library for creating [parsers](https://en.wikipedia.org/wiki/Parsing#Parser) according to a [Parsing Expression Grammar](http://en.wikipedia.org/wiki/Parsing_expression_grammar) (PEG).


## Documentation

* [Main Branch Documentation](https://github.com/taocpp/PEGTL/blob/main/doc/README.md) &nbsp; <sup>[C++23](https://en.cppreference.com/w/cpp/23.html)</sup>
* [Version 4.x Documentation](https://github.com/taocpp/PEGTL/blob/4.x/doc/README.md) &nbsp; <sup>[C++17](https://en.cppreference.com/w/cpp/17.html)</sup>
* [Version 3.x Documentation](https://github.com/taocpp/PEGTL/blob/3.x/doc/README.md) &nbsp; <sup>[C++17](https://en.cppreference.com/w/cpp/17.html)</sup>
* [Version 2.x Documnetation](https://github.com/taocpp/PEGTL/blob/2.x/doc/README.md) &nbsp; <sup>[C++11](https://en.cppreference.com/w/cpp/11.html)</sup>
* [Version 1.x Documentation](https://github.com/taocpp/PEGTL/blob/1.x/doc/README.md) &nbsp; <sup>[C++11](https://en.cppreference.com/w/cpp/11.html)</sup>


## Introduction

Grammars are written as regular C++ code, created with template programming (not template meta programming), i.e. nested template instantiations that naturally correspond to the inductive definition of PEGs (and other parser-combinator approaches).

A comprehensive set of [parser rules](doc/Rule-Reference.md) that can be combined and extended by the user is included, as are mechanisms for [debugging grammars](doc/Debug-Facilities.md), and for attaching user-defined [actions](doc/Actions-and-States.md) to grammar rules.
Here is an example of a parsing expression [grammar rule](doc/Rules-and-Grammars.md) implemented as a C++ class with the PEGTL.

```c++
// PEG rule for integers consisting of a non-empty
// sequence of digits with an optional sign:

// sign ::= '+' / '-'
// integer ::= sign? digit+

// The same parsing rule implemented with the PEGTL:

using namespace tao::pegtl;

struct sign : one< '+', '-' > {};
struct integer : seq< opt< sign >, plus< digit > > {};
```

PEGs are superficially similar to [Context-Free Grammars](https://en.wikipedia.org/wiki/Context-free_grammar) (CFGs), however the more deterministic nature of PEGs gives rise to some very important differences in semantics.
The included [grammar analysis](doc/Debug-Facilities.md#grammar-analysis) finds several typical errors in PEGs, including [left recursion](https://en.wikipedia.org/wiki/Left_recursion).


## Design

The PEGTL is designed to be "lean and mean", putting a well-tuned *relatively* simple approach above overly complex optimizations.
Grammars and parsing rules are expressed in C++ as template instantiations, burdening the compiler with a lot of the optimizations.

The PEGTL is mostly concerned with parsing combinators and grammar rules, and with giving the user of the library (the possibility of) full control over all other aspects of a parsing run.
Whether/which [actions](doc/Actions-and-States.md) are taken, and whether/which data structures are created during a [parsing run](doc/Inputs-and-Parsing.md), is entirely up to the user.

Included are some [examples](doc/Example-Reference.md) for typical situations like parsing integers, unescaping escape sequences in strings, building a generic [JSON](http://www.json.org/) data structure, and on-the-fly evaluation of arithmetic expressions.

Through the use of template programming and template specialisations it is possible to write a grammar once, and use it in multiple ways with different (semantic) actions in different (or the same) parsing runs.

With the PEG formalism, the separation into lexer and parser stages is usually dropped -- everything is done in a single grammar.
The PEGTL can however also be used to implement a tokenizer as well as for parsing sequences of arbitrary custom token objects.


## Quality

An extensive test suite is run automatically on each push, running on multiple architectures, operating system, compilers, and versions thereof.

The test suite is also run with the GCC and Clang [sanitizers](https://github.com/google/sanitizers), Clang's [Static Analyzer](https://clang-analyzer.llvm.org/), and [`clang-tidy`](http://clang.llvm.org/extra/clang-tidy/).
Additionally, [CodeQL](https://securitylab.github.com/tools/codeql) is used to scan for (security) issues.

[Releases](https://github.com/taocpp/PEGTL/releases) are done in accordance with [Semantic Versioning](http://semver.org/); incompatible changes to the public API are *only* allowed to occur with a new major version.

For releases the test suite covers 100% of the core library.
To make up for the imprecision of code coverage measurements on C++ code with this many templates, AI is also used to verify full test coverage.

For a stable experience please download [the latest release](https://github.com/taocpp/PEGTL/releases) instead of using the current state of development on the `main` branch.


## Contact

For questions and suggestions regarding the PEGTL, success or failure stories, and any other kind of feedback, please feel free to open a [discussion](https://github.com/taocpp/PEGTL/discussions), an [issue](https://github.com/taocpp/PEGTL/issues) or a [pull request](https://github.com/taocpp/PEGTL/pulls), or contact the authors at `taocpp(at)icemx.net`.


## Thank You

In appreciation of all contributions here are the people that have [directly contributed](https://github.com/taocpp/PEGTL/graphs/contributors) to the PEGTL and/or its development.

[<img alt="amphaal" src="https://avatars.githubusercontent.com/u/13903151" width="120">](https://github.com/amphaal)
[<img alt="anand-bala" src="https://avatars.githubusercontent.com/u/7420072" width="120">](https://github.com/anand-bala)
[<img alt="andoma" src="https://avatars.githubusercontent.com/u/216384" width="120">](https://github.com/andoma)
[<img alt="barbieri" src="https://avatars.githubusercontent.com/u/1838" width="120">](https://github.com/barbieri)
[<img alt="bjoe" src="https://avatars.githubusercontent.com/u/727911" width="120">](https://github.com/bjoe)
[<img alt="bwagner" src="https://avatars.githubusercontent.com/u/447049" width="120">](https://github.com/bwagner)
[<img alt="cdiggins" src="https://avatars.githubusercontent.com/u/1759994" width="120">](https://github.com/cdiggins)
[<img alt="clausklein" src="https://avatars.githubusercontent.com/u/1833050" width="120">](https://github.com/clausklein)
[<img alt="delpinux" src="https://avatars.githubusercontent.com/u/35096584" width="120">](https://github.com/delpinux)
[<img alt="dkopecek" src="https://avatars.githubusercontent.com/u/1353140" width="120">](https://github.com/dkopecek)
[<img alt="gene-hightower" src="https://avatars.githubusercontent.com/u/3957811" width="120">](https://github.com/gene-hightower)
[<img alt="irrequietus" src="https://avatars.githubusercontent.com/u/231192" width="120">](https://github.com/irrequietus)
[<img alt="jedelbo" src="https://avatars.githubusercontent.com/u/572755" width="120">](https://github.com/jedelbo)
[<img alt="joelfrederico" src="https://avatars.githubusercontent.com/u/458871" width="120">](https://github.com/joelfrederico)
[<img alt="johelegp" src="https://avatars.githubusercontent.com/u/21071787" width="120">](https://github.com/johelegp)
[<img alt="jovermann" src="https://avatars.githubusercontent.com/u/6087443" width="120">](https://github.com/jovermann)
[<img alt="jubnzv" src="https://avatars.githubusercontent.com/u/12023585" width="120">](https://github.com/jubnzv)
[<img alt="kelvinhammond" src="https://avatars.githubusercontent.com/u/1824682" width="120">](https://github.com/kelvinhammond)
[<img alt="kneth" src="https://avatars.githubusercontent.com/u/1225363" width="120">](https://github.com/kneth)
[<img alt="kuzmas" src="https://avatars.githubusercontent.com/u/1858553" width="120">](https://github.com/kuzmas)
[<img alt="lambdafu" src="https://avatars.githubusercontent.com/u/1138455" width="120">](https://github.com/lambdafu)
[<img alt="lichray" src="https://avatars.githubusercontent.com/u/433009" width="120">](https://github.com/lichray)
[<img alt="michael-brade" src="https://avatars.githubusercontent.com/u/8768950" width="120">](https://github.com/michael-brade)
[<img alt="mkrupcale" src="https://avatars.githubusercontent.com/u/13936020" width="120">](https://github.com/mkrupcale)
[<img alt="newproggie" src="https://avatars.githubusercontent.com/u/162319" width="120">](https://github.com/newproggie)
[<img alt="obiwahn" src="https://avatars.githubusercontent.com/u/741109" width="120">](https://github.com/obiwahn)
[<img alt="ohanar" src="https://avatars.githubusercontent.com/u/1442822" width="120">](https://github.com/ohanar)
[<img alt="pauloscustodio" src="https://avatars.githubusercontent.com/u/70773" width="120">](https://github.com/pauloscustodio)
[<img alt="pleroux0" src="https://avatars.githubusercontent.com/u/39619854" width="120">](https://github.com/pleroux0)
[<img alt="quadfault" src="https://avatars.githubusercontent.com/u/30195320" width="120">](https://github.com/quadfault)
[<img alt="quarticcat" src="https://avatars.githubusercontent.com/u/70888415" width="120">](https://github.com/quarticcat)
[<img alt="ras0219" src="https://avatars.githubusercontent.com/u/533828" width="120">](https://github.com/ras0219)
[<img alt="redmercury" src="https://avatars.githubusercontent.com/u/4424222" width="120">](https://github.com/redmercury)
[<img alt="robertcampion" src="https://avatars.githubusercontent.com/u/4220569" width="120">](https://github.com/robertcampion)
[<img alt="samhocevar" src="https://avatars.githubusercontent.com/u/245089" width="120">](https://github.com/samhocevar)
[<img alt="sanssecours" src="https://avatars.githubusercontent.com/u/691989" width="120">](https://github.com/sanssecours)
[<img alt="sgbeal" src="https://avatars.githubusercontent.com/u/235303" width="120">](https://github.com/sgbeal)
[<img alt="skyrich62" src="https://avatars.githubusercontent.com/u/23705081" width="120">](https://github.com/skyrich62)
[<img alt="studoot" src="https://avatars.githubusercontent.com/u/799344" width="120">](https://github.com/studoot)
[<img alt="svenjo" src="https://avatars.githubusercontent.com/u/1538181" width="120">](https://github.com/svenjo)
[<img alt="wickedmic" src="https://avatars.githubusercontent.com/u/12001183" width="120">](https://github.com/wickedmic)
[<img alt="wravery" src="https://avatars.githubusercontent.com/u/6502881" width="120">](https://github.com/wravery)
[<img alt="zhihaoy" src="https://avatars.githubusercontent.com/u/43971430" width="120">](https://github.com/zhihaoy)


## The Art of C++

The PEGTL is part of [The Art of C++](https://taocpp.github.io/).

[<img alt="colinh" src="https://avatars.githubusercontent.com/u/113184" width="120">](https://github.com/ColinH)
[<img alt="d-frey" src="https://avatars.githubusercontent.com/u/3956325" width="120">](https://github.com/d-frey)
[<img alt="uilianries" src="https://avatars.githubusercontent.com/u/4870173" width="120">](https://github.com/uilianries)


## License

<a href="https://opensource.org/licenses/BSL-1.0"><img align="right" src="https://opensource.org/wp-content/uploads/2009/06/OSIApproved.svg" width="150" hspace="20" alt="Open Source Initiative"></a>

Copyright (c) 2007-2026 Dr. Colin Hirsch and Daniel Frey

The PEGTL is certified [Open Source](http://www.opensource.org/docs/definition.html) software.
It is [licensed](https://pdimov.github.io/blog/2020/09/06/why-use-the-boost-license/) under the terms of the [Boost Software License, Version 1.0](https://www.boost.org/LICENSE_1_0.txt) reproduced here.

> Boost Software License - Version 1.0 - August 17th, 2003
>
> Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute, and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the Software is furnished to do so, all subject to the following:
>
> The copyright notices in the Software and this entire statement, including the above license grant, this restriction and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all derivative works of the Software, unless such copies or derivative works are solely in the form of machine-executable object code generated by a source language processor.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
