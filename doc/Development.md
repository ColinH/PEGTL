# Development

Public notes on the past, present and future development of the PEGTL.

### C++ Standard

Version 0.x of the PEGTL requires at least C++11.

Version 1.x of the PEGTL requires at least C++11.

Version 2.x of the PEGTL requires at least C++11.

Version 3.x of the PEGTL requires at least C++17.

Version 4.x of the PEGTL requires at least C++17.

Version 5.x of the PEGTL requires at least C++23.

### C++ Features

The C++20 modernization uses concepts for optional interfaces and overload selection, string-literal template arguments for the ASCII string rules, and callable template arguments including lambdas for `function`.
Inputs also support spans, and iterator readers accept separate sentinels and move-only input iterators.

Further opportunities include:

 * Add support for C++20 `char8_t` where appropriate.
 * Extend the use of C++20 `std::span` where it makes sense.
 * Investigate whether there is anything useful we can do with Ranges in the PEGTL.
 * ~~Keep an open eye for opportunities to use C++20 spaceship operator. Spaceship!~~
 * Keep an open eye for opportunities to use C++20 defaulted comparison operators.
 * Keep an open eye for opportunities to use C++20 `[[likely]]` and `[[unlikely]]`.
 * Keep an open eye for opportunities to use C++20 `constinit` and `consteval`, and
 * keep an open eye for opportunities to use the extended `constexpr` facilities.
 * For example make the bulk() and size() functions in the peek classes consteval!
 * Keep an open eye for opportunities to use the extended CTAD facilities from C++20.
 * Keep an open eye for opportunities to use class types as non-type template parameters.
 * Investigate whether we can use C++20 `std::bit_cast` to improve some of the low-level code.
 * Use C++23 "deducing this" feature to let base class `make_rewind_guard()` return a rewind guard for a derived class.
 * When can we assume the C++17 `charconv` facilities are universally available?
 * Auto-detect internal rules insead of internal::enable_control?

### Other Things

 * Investigate whether we are crazy enough to implement parsing non-contiguous memory like linked lists?
 * Investigate whether we are crazy enough to implement parsing non-linear data structures like trees?

### Stream Parsing

A couple of things that *could* be done in the area of [stream parsing](Stream-Parsing.md).

 * Use the double-mapped ring-buffer to prevent `discard()` having to move data.
 * Debug input and related facilities that detect when data in the input buffer is accessed after being discarded or moved (by a discard).
 * Investigate the use of ("stackful") coroutines for parsing from a network socket, and
 * investigate whether this can also be used for stream parsing that keeps everything.

### Development Tools

We use the code coverage facilities of the GCC and Clang compilers to meet our goal of 100% code coverage with unit tests.
Unfortunately the coverage reports are not 100% reliable.

We also try to use `clang-format` to ensure consistent formatting of our source code.
Unfortunately it, too, can get confused and result in incorrectly formatted code.

For example under some circumstances `clang-format` does not understand whether a `&&` is the logical operator or an r-value reference declarator, or whether a `<` is a comparison operator or introduces a template argument list.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
