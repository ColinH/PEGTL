# Object Parsing

The PEGTL can parse more than just sequences of characters, it can also parse binary data and arbitrary objects.


## Contents

* [Overview](#overview)
* [Binary and Enums](#binary-and-enums)
* [Arbitrary Objects](#arbitrary-objects)
  * [Object Inputs](#object-inputs)
  * [Member Rules](#member-rules)
  * [Custom Object Rules](#custom-object-rules)
  * [Nested Object Parsing](#nested-object-parsing)
  * [Examples](#examples)


## Overview

The PEGTL treats an input as a contiguous sequence of objects of type `ParseInput::data_t`.
Most examples use `char`, however the same parsing machinery also works for other simple data types, binary integer encodings, and token-like objects.

Two common use cases are:

* parsing binary data where several input bytes represent one integer or enum value, and
* parsing a token stream, usually a `std::vector` of small structs created by a separate lexer.

The important limitation is the same as for all PEGTL inputs: the input data must be contiguous in memory.
For example `std::vector`, `std::array`, and `std::string` can be used, while `std::list` and `std::deque` cannot.


## Binary and Enums

There are [rules dedicated](Rule-Reference.md#binary) to parsing binary data in the form of integers and enums.
They are not included by `<tao/pegtl.hpp>` and have to be included individually.

```c++
#include <tao/pegtl/binary/uint16.hpp>

struct magic
   : tao::pegtl::uint16_be::one< 0x5047 >
{};
```

The binary integer rules live in namespaces that encode both size and byte order, for example `uint16_be` and `uint16_le`.
Namespace aliases that point to the corresponding native-endian namespace are also provided, e.g. namespace alias `uint16` that aliases either `uint16_be` or `uint16_le` depending on the platform's byte order.

The enum rules are available as `enums_be`, `enums_le`, and native-endian `enums`.
All non-type template parameters of an enum rule must have the same enum type.

Binary rules can parse inputs whose `data_t` is an integral or enum type with a suitable object size.
For a rule that parses an N-byte integer or enum value, the input object size must be either 1 byte or N bytes.
For example a `uint16_be::one< 0x0102 >` rule consumes two objects from an 8-bit input and one object from a 16-bit input.

This means byte-oriented data from files, strings, or memory can be parsed with the usual `char`-based inputs using binary or enum rules of any size.

If only parsing rules for, say, 16-bit integers and/or enums are used, an input with a 16-bit datatype can be chosen instead.

```c++
std::vector< std::uint16_t > data = { 0x0102, 0x0304 };

tao::pegtl::view_input< void, std::uint16_t > in( data );
tao::pegtl::parse< tao::pegtl::seq<
   tao::pegtl::uint16::one< 0x0102 >,
   tao::pegtl::uint16::one< 0x0304 >,
   tao::pegtl::eof
> >( in );
```

The main difference is that object counts and offsets are now in terms of 16-bit, i.e. 2-byte, units which can be an advantage when only dealing with objects of that size.

## Arbitrary Objects

Parsing arbitrary objects usually means parsing a sequence of tokens.
A lexer first produces a contiguous container of token objects, and a second PEGTL grammar parses that token stream.

The object grammar can still use the normal combinators such as [`seq`](Rule-Reference.md#seq-r-), [`sor`](Rule-Reference.md#sor-r-), [`plus`](Rule-Reference.md#plus-r-), [`list`](Rule-Reference.md#list-r-s-), [`must`](Rule-Reference.md#must-r-), and [`eof`](Rule-Reference.md#eof).
Only the leaf rules that inspect a token are different, i.e. must be specific to the tokens (or other objects) being parsed.

### Object Inputs

The usual `view_input` and `copy_input` can be chosen depending on whether the token container outlives the parsing run or a copy is needed.
For a token stream the `Eol` template parameter is usually `void` because [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) do not make sense for arbitrary token objects.
Should a specific `Eol` token exist the end-of-line rule can of course be set up to recognize it.

```c++
enum class token_type
{
   number,
   plus,
   minus
};

struct token
{
   token_type type;
   std::string text;
};

using token_view = tao::pegtl::view_input< void, token >;
using token_copy = tao::pegtl::copy_input< void, std::vector< token > >;
```

All ordinary input operations still operate in token units.
For example `in.size()` returns the number of remaining tokens, `in.current()` points to the next token, `in.consume< R >( 1 )` consumes one token, and an action input's `peek()` returns a `const token&`.

The convenience functions `string()` and `string_view()` require character data accessible as `const char*`.
For token inputs the `peek()`, `current()`, `span()` or `vector()` accessors are more useful.
The `vector()` accessor requires copy-constructible tokens; `span()` provides a view without copying them.

### Member Rules

The member rules in `<tao/pegtl/member.hpp>` apply one of the usual terminal-rule tests to a value extracted from the next input object.
They are described in detail in the [member rule reference](Rule-Reference.md#member).

```c++
#include <tao/pegtl/member.hpp>

template< token_type Type >
struct token_is
   : tao::pegtl::member::one< &token::type, Type >
{};

struct expression
   : tao::pegtl::seq<
        token_is< token_type::number >,
        tao::pegtl::star<
           tao::pegtl::sor<
              token_is< token_type::plus >,
              token_is< token_type::minus >
           >,
           token_is< token_type::number >
        >,
        tao::pegtl::eof
     >
{};
```

The first template parameter, `M`, tells the rule how to extract a value from the next input object.
It can be a pointer to a data member, a pointer to a suitable const member function, or a pointer to a suitable free getter function.
Member getters qualified with `const &` are also supported.
Value results are stored in the peek result; reference and pointer results refer to existing objects, which must remain alive while the result is used.
A null pointer means no value is available for matching.

The extracted value is then tested by one of the member rules:

* `member::one< M, U... >`, `member::not_one< M, U... >`
* `member::range< M, U, V >`, `member::not_range< M, U, V >`
* `member::ranges< M, U... >`, `member::not_ranges< M, U... >`
* `member::string< M, U... >`
* `member::function< M, F >`
* `member::nested< M, R >`

Each successful member rule consumes one input object, except `member::string` which consumes one object per listed value.
On local failure the input is left unchanged when rewinding is required, just like for the corresponding character rules.

The predicate `F` supplied to `member::function< M, F >` must return a result implicitly convertible to `bool` to signal parsing success or failure.
As with [`function`](Rule-Reference.md#function-f-p--void-), it can be a function pointer, a lambda, a structural callable object or a member pointer.
It is invoked with the extracted value and all state objects when supported, otherwise with only the extracted value.
For example a member-function pointer can test a token's text:

```c++
struct empty_text
   : tao::pegtl::member::function< &token::text, &std::string::empty >
{};
```

### Custom Object Rules

For matching conditions that are not covered by the member rules, implement a normal custom rule.
This is sometimes useful when a rule has to inspect several fields of the token, or when the decision depends on parsing state.

```c++
struct token_check
{
   using rule_t = token_check
   using subs_t = tao::pegtl::empty_list;

   template< typename ParseInput >
   static bool match( ParseInput& in )
   {
      if( !in.empty() ) {
         if( some_global_check( in.current()->text ) ) {
            in.template consume< token_check >( 1 );
            return true;
         }
      }
      return false;
   }
};
```

The same custom-rule protocol as for character inputs applies.
In particular, returning `false` is a local failure and must not consume input.

Custom rules that should participate in [grammar analysis](Debug-Facilities.md#grammar-analysis) might also need an `analyze_traits` specialization.
For a leaf rule that consumes one object on success, `analyze_any_traits<>` is usually appropriate.

### Nested Object Parsing

The `member::nested< M, R >` rule extracts an object from the next token and parses it with grammar `R`.
The extracted object must be suitable for constructing a [`view_input`](Input-Reference.md#view-input), for example a `std::string` or `std::vector`.

```c++
struct word
   : tao::pegtl::seq< tao::pegtl::alpha, tao::pegtl::star< tao::pegtl::alnum >, tao::pegtl::eof >
{};

struct word_token
   : tao::pegtl::member::nested< &token::text, word >
{};
```

The outer token is consumed when the inner parsing run succeeds.
If the inner parsing run fails locally, the outer rule also fails locally and no token is consumed.

### Examples

The following examples show the patterns above in context.

* [`src/example/token_input_1.cpp`](../src/example/token_input_1.cpp) defines a manual token-matching rule.
* [`src/example/token_input_2.cpp`](../src/example/token_input_2.cpp) implements the same token grammar with `member::one`.
* [`src/example/json_tokens.cpp`](../src/example/json_tokens.cpp) shows a two-stage JSON lexer and token parser.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
