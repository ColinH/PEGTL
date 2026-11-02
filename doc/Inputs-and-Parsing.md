# Inputs and Parsing

**Input data** is a sequence of objects -- often of type `char` -- that is intended to be parsed.

An **input** is a class that adheres to an informal interface and represents some input data.


## Contents

* [Introduction](#introduction)
* [Input Classes](#input-classes)
  * [Classification](#classification)
  * [Parameters](#parameters)
* [Ends of Lines](#ends-of-lines)
  * [Rule Tracking](#rule-tracking)
  * [Scan Tracking](#scan-tracking)
  * [Lazy Tracking](#lazy-tracking)
* [Parse Function](#parse-function)
* [Nested Parsing](#nested-parsing)
* [Position Classes](#position-classes)
* [Input Interface](#input-interface)
  * [Basic Interface](#basic-interface)
  * [Inputs with Start](#inputs-with-start)
  * [Inputs with Lines](#inputs-with-lines)
  * [Inputs with Source](#inputs-with-source)
  * [Input Convenience](#input-convenience)
  * [Stream Compatibility](#stream-compatibility)


## Introduction

Performing a parsing run requires (at least) the following steps.

1. The [grammar](Rules-and-Grammars.md) has to be defined.
2. An [input](#input-classes) has to be constructed.
3. The [parse function](#parse-function) has to be called with the grammar and the input.

The following steps are also frequently included to do something useful while parsing.

4. [Actions](Actions-and-States.md#introduction) have to be implemented and passed to the parse function as template parameter.
5. [States](Actions-and-States.md#states) have to be instantiated and passed to the parse function as additional arguments.

More advanced use cases might also pass a [control class](Control-and-Normal.md) to the parsing run, or use other functions to drive the parsing run.

The following incomplete code shows the general outline of performing a parsing run.

```c++
using namespace tao::pegtl;

// A state class to collect data while parsing:

class my_state;

// Implementation of required grammar rules:

struct some_rule : ... {};

struct my_grammar : ... {};

// Action class with default case that does nothing:

template< typename Rule >
struct my_actions
   : tao::pegtl::nothing< Rule >{};

// Specializations of my_actions as required for rules:

template<>
struct my_actions< some_rule >
{
   template< class ActionInput >
   static void apply( const ActionInput& in, my_state& state )
   { ... }
};

// Putting everything together to start parsing:

[[nodiscard]] bool my_parse( const std::filesystem::path& file, my_state& state )
{
   file_input in( file );
   return parse< my_grammar, my_actions >( in, state );
}
```


## Input Classes

The PEGTL includes several [input classes](Input-Reference.md) for parsing memory, standard library containers, and files.
Additionally there are dedicated [stream inputs](Stream-Parsing.md#inputs) for [stream parsing](Stream-Parsing.md).
The [stream inputs](Stream-Parsing.md#inputs) are **only** documented on the page dedicated to [stream parsing](Stream-Parsing.md).

All inputs are class templates that can be customized.
Let us classify the inputs according to their names, explaining what e.g. a `text_mmap_input` is, and then document their template parameters.

### Classification

The inputs for data in memory are either

- `view` inputs that keep a reference (pointer and size) without taking ownership, or
- `copy` inputs that make a copy of the data in a private container like `std::string`.

The `argv` and `base` inputs are also `view` inputs according to this classification.

The `copy` inputs are move aware when presented with an r-value reference during construction.

For all inputs the data **must** reside in a contiguous piece of memory, i.e. a `std::vector` or `std::array` can be used, but **not** a `std::list` or `std::deque`.

The inputs for data in a file are either

- `read` inputs that use `std::fread()` to read the file into memory upon construction, or
- `mmap` inputs that use `::mmap(2)` (or similar) to map the file into the address space, or
- `file` inputs that are built with an `mmap` input when available, and a `read` input as fallback.

All of the above inputs are also either

- plain inputs whose position information is "simple", most often a count from the start of the input data, or
- `text` inputs whose position information includes a line and column number based on the `Eol` parameter explained below.

> [!NOTE]
> The column number is just an offset from the start of the input data.
> It does **not** take into account that a single [Unicode](https://en.wikipedia.org/wiki/Unicode) character can be composed of multiple code points, and a single code point can be encoded in multiple code units.

In addition there is the [`action_input`](Actions-and-States.md#action-input) which is related to the inputs listed here but falls into a slightly different category since it can not be used as an input for a parsing run.

### Parameters

The `typename Eol` template parameter determines which (sequence of) object(s) constitute an end-of-line.

When this parameter is set to `void` the [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) rules can not be used, and neither can the `begin_of_line()`, `end_of_file_or_line()` and `line_view_at()` input member functions.

All input classes except for `argv_input` use the default end-of-line rule as default for the `Eol` template parameter.

On Windows the default end-of-line rule is [`tao::pegtl::(ascii::)scan::lf_crlf`](Rule-Reference.md#lf_crlf) which matches both Unix and MS-DOS line endings.

On Unix and Linux, including macOS and Android, the default end-of-line rule is [`tao::pegtl::(ascii::)scan::lf`](Rule-Reference.md#lf) which matches Unix line endings.

For an explanation of the three tracking modes and tables of the included end-of-line rules see the [ends of lines](#ends-of-lines) section below.

The `typename Data` template parameter determines the type of objects in the [input data](Introduction.md#definitions).
For inputs that have this template parameter it defaults to `char` and can be changed, for all others it is hard-wired to `char`.

The `typename Source` template parameter determines the data type of an optional fixed part of the position information.
The name "source" comes from the most frequent use case, the source filename.
For inputs that have this template parameter it defaults to `void`, no source information is stored.
For all filesystem inputs this is hard-wired to `std::filesystem::path` because error messages like "parse error in line 10" *without* the filename have made me want to throw my computer out the window on too many occasions.

Many inputs have two source types, `InputSource` and `ErrorSource`.
In this case the source is stored as type `InputSource` in the input, but as type `ErrorSource` in thrown `parse_error_base`-derived exceptions.
A common use case for the separation is to use `std::string_view` as `InputSource` and `std::string` as `ErrorSource`; cheap view in the input, more expensive copy in the exception to make it self-contained.

The `typename Container` template parameter determines the container in which `copy` inputs keep the input data.
It defaults to `std::string` but can be changed to match an already existing object to enable moving.


## Ends of Lines

There are three ways `text` inputs can use the `Eol` rule to track or calculate the line and column numbers during a parsing run.
Rule and scan tracking are both eager while lazy tracking is lazy.

### Rule Tracking

Rule tracking continuously updates the line and column numbers during a parsing run.

The column number is updated with every successful rule match.
The line number is **only** updated when the [`eol`](Rule-Reference.md#eol) or [`eolf`](Rule-Reference.md#eolf) rules match.

- Low overhead while parsing, but
- care has to be taken to not accidentally match the character(s) (or object(s)) constituting an end-of-line with a rule that is **not** `eol` or `eolf`.
- There are no limitations or requirements for the `Eol` rule.

Rules that enable rule tracking are just any normal rules in the ASCII and Unicode namespaces (`tao::pegtl::ascii`, `tao::pegtl::utf8` etc.)

| Class | ASCII | Unicode |
| ----- | ----- | ------- |
| `...::cr` | [rule](Rule-Reference.md#cr) | [rule](Rule-Reference.md#cr-1) |
| `...::lf` | [rule](Rule-Reference.md#lf) | [rule](Rule-Reference.md#lf-1) |
| `...::crlf` | [rule](Rule-Reference.md#crlf) | [rule](Rule-Reference.md#crlf-1) |
| `...::cr_lf` | [rule](Rule-Reference.md#cr_lf) | [rule](Rule-Reference.md#cr_lf-1) |
| `...::cr_crlf` | [rule](Rule-Reference.md#cr_crlf) | [rule](Rule-Reference.md#cr_crlf-1) |
| `...::lf_crlf` | [rule](Rule-Reference.md#lf_crlf) | [rule](Rule-Reference.md#lf_crlf-1) |
| `...::cr_lf_crlf` | [rule](Rule-Reference.md#cr_lf_crlf) | [rule](Rule-Reference.md#cr_lf_crlf-1) |
| `...::ls` | - | [rule](Rule-Reference.md#ls) |
| `...::nel` | - | [rule](Rule-Reference.md#nel) |
| `...::ps` | - | [rule](Rule-Reference.md#ps) |
| `...::eol1` | - | [rule](Rule-Reference.md#eol1) |
| `...::eolu` | - | [rule](Rule-Reference.md#eolu) |

The table shows the most commonly used end-of-line rules, however anything (outside of the `scan` and `lazy` sub-namespaces) can be used.

This tracking mode was introduced in PEGTL 4.0.

### Scan Tracking

Scan tracking also continuously updates the line and column numbers during a parsing run.

After every successful rule match the matched portion of the input is scanned for any occurrences of an end-of-line.

- Some overhead while parsing, and
- only works for end-of-line rules with a designated code point that signifies end-of-line, but
- not a problem if a rule distinct from [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) matches an end-of-line.

Rules that enable scan tracking can be found in the `scan` sub-namespace of the ASCII and Unicode namespaces (`tao::pegtl::ascii`, `tao::pegtl::utf8` etc.)

| Class | ASCII | Unicode |
| ----- | ----- | ------- |
| `...::scan::cr` | [rule](Rule-Reference.md#cr) | [rule](Rule-Reference.md#cr-1) |
| `...::scan::lf` | [rule](Rule-Reference.md#lf) | [rule](Rule-Reference.md#lf-1) |
| `...::scan::lf_crlf` | [rule](Rule-Reference.md#lf_crlf) | [rule](Rule-Reference.md#lf_crlf-1) |
| `...::scan::ls` | - | [rule](Rule-Reference.md#ls) |
| `...::scan::nel` | - | [rule](Rule-Reference.md#nel) |
| `...::scan::ps` | - | [rule](Rule-Reference.md#ps) |

This tracking mode corresponds to the eager tracking in PEGTL versions prior to 4.0.

Note that the scan is skipped if it can be statically proven that the matched input does not contain an end-of-line, for example in the case of a `tao::pegtl::(ascii::)string` where all characters in the string are **not** the designated end-of-line character.

### Lazy Tracking

Lazy tracking does not continuously update the line and column numbers during a parsing run.

The position information is calculated on demand, i.e. when `current_position()` or `previous_position()` are called on the input.
In that case an eol scan is performed on the input data from the start to the point for which position information was requested.

- Zero overhead while parsing, but
- linear complexity in size of input data to calculate line and column number on demand.
- Not a problem if a rule distinct from [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) matches an end-of-line.
- There are no limitations or requirements for the `Eol` rule, except:
- it needs to define an appropriate`eol_lazy_peek` type alias.

This type alias is used by the eol scan to skip to the next place in the input data at which to attempt an `Eol` match.

Rules that enable lazy tracking can be found in the `lazy` sub-namespace of the ASCII and Unicode namespaces (`tao::pegtl::ascii`, `tao::pegtl::utf8` etc.)

| Class | ASCII | Unicode |
| ----- | ----- | ------- |
| `...::lazy::cr` | [rule](Rule-Reference.md#cr) | [rule](Rule-Reference.md#cr-1) |
| `...::lazy::lf` | [rule](Rule-Reference.md#lf) | [rule](Rule-Reference.md#lf-1) |
| `...::lazy::crlf` | [rule](Rule-Reference.md#crlf) | [rule](Rule-Reference.md#crlf-1) |
| `...::lazy::cr_lf` | [rule](Rule-Reference.md#cr_lf) | [rule](Rule-Reference.md#cr_lf-1) |
| `...::lazy::cr_crlf` | [rule](Rule-Reference.md#cr_crlf) | [rule](Rule-Reference.md#cr_crlf-1) |
| `...::lazy::lf_crlf` | [rule](Rule-Reference.md#lf_crlf) | [rule](Rule-Reference.md#lf_crlf-1) |
| `...::lazy::cr_lf_crlf` | [rule](Rule-Reference.md#cr_lf_crlf) | [rule](Rule-Reference.md#cr_lf_crlf-1) |
| `...::lazy::ls` | - | [rule](Rule-Reference.md#ls) |
| `...::lazy::nel` | - | [rule](Rule-Reference.md#nel) |
| `...::lazy::ps` | - | [rule](Rule-Reference.md#ps) |
| `...::lazy::eolu` | - | [rule](Rule-Reference.md#eolu) |

This tracking mode was extended in PEGTL 4.0 -- previously lazy tracking had the same restrictions as scan tracking.

Note that the case of simple end-of-line rules, i.e. `tao::pegtl::(ascii::)one< N >` for a single `N` uses a slightly more optimized scan.


## Parse Function

The `parse()` function is the single most important user-facing function, it starts a [parsing run](Introduction.md#definitions).

```c++
template< typename Rule,
          template< typename... > class Action = nothing,
          template< typename... > class Control = normal,
          apply_mode A = apply_mode::enabled,
          rewind_mode M = rewind_mode::optional,
          typename ParseInput,
          typename... States >
bool parse( ParseInput&& in,
            States&&... st );
```

- The [`Rule` class](Rules-and-Grammars.md) represents the top-level parsing rule of the grammar and is mandatory.
- The [`Action`](Actions-and-States.md) defaults to an action that does nothing. It is required to pass a user-defined action for a parsing run to do more, e.g. build some data structure, than validate an input against the grammar.
- The [`Control`](Control-and-Normal.md) defaults to the normal control class that implements the expected and documented behavior. It can be changed for debugging, e.g. printing all rule match attempts and their outcomes, and for some other advanced use cases, e.g. gathering rule invocation statistics.
- The [`States`](Actions-and-States.md#changing-states) are the types of the additional state objects `st` that are passed to all rules' `match()` functions, all actions' `apply()` and `apply0()` functions, and all control functions. What is needed here depends on what the actions (and control functions) expect.
- The `apply_mode` defaults to `apply_mode::enabled` which enables actions. Can be changed to `apply_mode::disabled` or in the grammar with the [`enable`](Rule-Reference.md#enable-r-) and [`disable`](Rule-Reference.md#disable-r-) rules.
- The `rewind_mode` defaults to `rewind_mode::optional` in which case the input might not be rewound to its start when `parse()` returns `false`. Rewinding can be enabled by passing `rewind_mode::required`.

The input being parsed must not be `const`; this also applies to the inner input passed to `parse_nested()`.

A parsing run can have the [same three outcomes](Rules-and-Grammars.md#match-function) as the match function of a rule.
Note that the distinction between "local" and "global" failure does not make too much sense at top-level, however for sake of consistency we will use these terms in all appropriate contexts.

- *success*, a return value of `true`,
- *local failure*, a return value of `false`,
- *global failure*, an exception, often derived from `tao::pegtl::parse_error_base`.

The `parse()` function itself does not add any further structure to the top-level rule.
In particular, `parse< Rule >( in )` succeeds as soon as `Rule` succeeds; it does **not** require the input to be exhausted.
Use a grammar such as `seq< Rule, eof >` to make unconsumed input a local failure.
Use `must< Rule, eof >` when the top-level parse should either succeed or throw, or `seq< Rule, must< eof > >` when only a failure of `eof` should be converted to a global failure.

The `parse()` function also does not catch exceptions.
Global failures created by PEGTL rules or by the default `normal` control throw exceptions derived from `tao::pegtl::parse_error_base`, usually the concrete `parse_error_t` of the input.
Exceptions thrown by input operations, actions, custom rules, or custom controls propagate unchanged unless the grammar or caller catches them.


## Nested Parsing

Nested parsing refers to an (inner) parsing run that is performed during another (outer) parsing run, for example when a file being parsed includes another file.

When a `std::exception` is thrown within a nested parsing run it will be caught by `tao::pegtl::parse_nested()` and a new exception thrown via `Control< Rule >::raise_nested()`.
The new exception contains a position from the argument of type `OuterInput` and the previous exception as nested exception.
Exceptions that do not derive from `std::exception` are not caught.

The functions in the header [`tao/pegtl/extra/nested_exceptions.hpp`](Extra-Reference.md#nested_exceptionshpp) can be used to work with these nested exceptions.
The inner-most exception that was thrown first will be the "most nested" exception, i.e. the final one in the linked list of nested exceptions.

The position information contained in the nested exceptions allows for error messages like "error in file F1 line L1 included from file F2 line L2 etc."

Calling `parse_nested()` requires the input from the outer parsing run, or the position within the outer parsing run, as additional first argument ("additional" as compared to `parse()`).

```c++
template< typename Rule,
          template< typename... > class Action = nothing,
          template< typename... > class Control = normal,
          apply_mode A = apply_mode::enabled,
          rewind_mode M = rewind_mode::optional,
          typename OuterInput,
          typename ParseInput,
          typename... States >
bool parse_nested( const OuterInput& oi,
                   ParseInput&& in,
                   States&&... st );
```

The `OuterInput` will usually be the input from the outer parsing run; it can also be the position obtained from that input.
More precisely, if `oi.current_position()` is **not** callable then `oi` is assumed to be a position itself, otherwise it is called to obtain a position.


## Position Classes

Positions occur as the return type of the input functions `current_position()` and `previous_position()`, and as the template parameter, and therefore position object, of `tao::pegtl::parse_error< Position >` exceptions.

All `text` inputs use `tao::pegtl::text_position` for their position reporting; when the error source type is not `void` the type is `tao::pegtl::position_with_source< ErrorSourceType, tao::pegtl::text_position >`.
For all filesystem inputs the error source type is `std::filesystem::path`, for all other inputs it defaults to `void`.
The header `tao/pegtl/text_position_with_source.hpp` also defines the convenience aliases `text_position_with_source` for `position_with_source< std::string, text_position >` and `text_position_with_path` for `position_with_source< std::filesystem::path, text_position >`.

Most non-`text` inputs use `tao::pegtl::count_position` for their position reporting; when the error source type is not `void` the type is `tao::pegtl::position_with_source< ErrorSourceType, tao::pegtl::count_position >`.
The exceptions are the `base` inputs which are so basic that they neither keep track nor can compute the number of objects from the start of the input data, they use `tao::pegtl::pointer_position` instead.
For all filesystem inputs the error source type is `std::filesystem::path`, for all other inputs it defaults to `void` except for the `argv` input which defaults to `std::string`.


## Input Interface

All input classes adhere to an informally defined interface of which some parts are optional.
Some rules or other facilities will not function when the optional interface parts they rely on are not present.

### Basic Interface

The basic interface implemented by all inputs.

```c++
   using namespace tao::pegtl;

   using data_t = char ... or something else;
   using error_position_t = ...one of the position classes;
   using offset_position_t = void;
   using rewind_position_t = ...one of the position classes;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   [[nodiscard]] bool empty() const noexcept;
   [[nodiscard]] std::size_t size() const noexcept;  // Number of unconsumed input objects.

   [[nodiscard]] const data_t* current( const std::size_t offset = 0 ) const noexcept
   [[nodiscard]] const data_t* end() const noexcept;

   [[nodiscard]] const data_t* previous( const rewind_position_t saved ) const noexcept;
   [[nodiscard]] const data_t* previous( const error_position_t saved ) const noexcept;

   template< typename Rule >
   void consume( const std::size_t count ) noexcept;

   [[nodiscard]] rewind_position_t rewind_position() const noexcept;
   void rewind_to_position( const rewind_position_t saved ) noexcept;

   [[nodiscard]] error_position_t current_position() const noexcept;
   [[nodiscard]] error_position_t previous_position( const rewind_position_t saved ) const noexcept;
```

### Inputs with Start

An *input with start* is an input that remembers the initial return value of `current()` and can be restarted from that position.
Most inputs are inputs with start except for [`base_input`](Input-Reference.md#base-input) -- and [all stream inputs](Stream-Parsing.md#inputs).

```c++
   [[nodiscard]] const data_t* start() const noexcept;

   void restart() noexcept;
```

### Inputs with Lines

An *input with lines* defines an `eol_rule` type alias which enables use of the [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) rules.

Note that an input with lines does not necessarily include line and column numbers in its position tracking, that is only provided by [`text` inputs](#classification).

Note that when one of the user-facing input classes is given `void` as `Eol` template parameter it disables the `eol_rule` type alias and is **not** considered an *input with lines*.

For an in-depth explanation of the choices regarding the `Eol` template parameter please see the [ends of lines](#ends-of-lines) section above.

```c++
   using eol_rule = Eol;  // template parameter, usually defaults to default_eol
```

Inputs with lines also implement the following functions that rely on the presence of `eol_rule`.

```c++
   [[nodiscard]] const data_t* begin_of_line( const error_position_t&, const std::size_t max = 135 ) const noexcept;
   [[nodiscard]] const data_t* end_of_line_or_file( const error_position_t&, const std::size_t max = 135 ) const;
```

### Inputs with Source

An *input with source* keeps an object that is part of the position but does not change over the parsing run.
For inputs that read from a file the source is the filename in a `std::filesystem::path`.

There are two source type aliases, `input_source_t` is the type of the source object embedded in the input, and `error_source_t` is the type of the source object embedded in the `error_position_t` which will be some `position_with_source<>` that is also used in the `parse_error< error_position_t >` exceptions.

> [!Note]
> If either of `input_source_t` or `error_source_t` is `void` then both must be `void`.

> [!Note]
> It must be possible to construct an `error_source_t` from a `const input_source_t&`.

```c++
   using input_source_t = ...std::string or user chosen;
   using error_source_t = ...std::string or user chosen;

   [[nodiscard]] const input_source_t& direct_source() const noexcept;
```

### Input Convenience

The input wrappers, including `action_input`, provide the following convenience functions when the underlying input and `data_t` support the required operations.

```c++
   [[nodiscard]] const data_t& peek( const std::size_t offset = 0 ) const noexcept;

   template< typename T >
   [[nodiscard]] T peek_as( const std::size_t offset = 0 ) const noexcept;

   [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept;
   [[nodiscard]] std::byte peek_byte( const std::size_t offset = 0 ) const noexcept;
   [[nodiscard]] std::int8_t peek_int8( const std::size_t offset = 0 ) const noexcept;
   [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept;

   [[nodiscard]] std::string string() const;
   [[nodiscard]] std::string_view string_view() const noexcept;
   [[nodiscard]] std::span< const data_t > span() const noexcept;
   [[nodiscard]] std::vector< data_t > vector() const;

   template< typename Position >
   [[nodiscard]] std::string_view line_view_at( const Position& pos );
```

The `peek()` function returns the object at `offset` without checking bounds.
The caller must ensure that this object exists.

The remaining functions have the following requirements:

| Functions | Requirements |
| --- | --- |
| `peek_as< T >()` | `T` and `data_t` have the same size, and `static_cast< T >( value )` is valid for a `const data_t& value`. |
| `peek_char()`, `peek_byte()`, `peek_int8()`, `peek_uint8()` | The corresponding `peek_as< T >()` operation is available. |
| `string()`, `string_view()` | `current()` yields a pointer convertible to `const char*`, and `size()` is callable on the const input without an argument. |
| `span()` | `current()` and `size()` describe a contiguous range on the const input, with no argument required by `size()`. |
| `vector()` | `data_t` is cv-unqualified and copy-constructible, and `size()` is callable on the const input without an argument. |
| `line_view_at( pos )` | The input provides `begin_of_line( pos )` and `end_of_line_or_file( pos )` returning pointers convertible to `const char*`. |

The `line_view_at()` function returns a `std::string_view` of the line containing `pos`.
These line operations are available on suitable parse inputs, but are not forwarded by `action_input`.
The `string()` and `vector()` functions copy the range; `string_view()` and `span()` refer to the existing storage.

#### Span Views

The `span()` accessor returns a `std::span< const data_t >` over the remaining input.
On an `action_input` it instead refers to the complete range matched by the rule.
It does not copy the objects, and can therefore also be used with non-copyable input objects.

```c++
const char data[] = "hello";
tao::pegtl::view_input<> in( std::span< const char >( data, 5 ) );
const auto remaining = in.span();  // Five characters; the terminating null is excluded.
```

A span does not keep its underlying storage alive.
Destroying or changing that storage can invalidate it; for action inputs over a stream this includes discarding or moving buffered data.

### Stream Compatibility

The PEGTL is designed to minimize the impact of the existence of the [stream parsing](Stream-Parsing.md) on the core library.
This goal was *mostly* achieved with the exception of some input functions and how the rules use them.
All non-stream input classes implement the following functions for compatibility with the stream inputs.

```c++
   [[nodiscard]] decltype( auto ) end( const std::size_t /*unused*/ ) const noexcept( auto )
   {
      return end();
   }

   [[nodiscard]] std::size_t size( const std::size_t /*unused*/ ) const noexcept( auto )
   {
      return size();
   }

   void require( const std::size_t /*unused*/ ) const noexcept
   {}

   void discard() const noexcept
   {}
```

All rules that need to be compatible with [stream inputs](Stream-Parsing.md#inputs) need to use the `end()` and `size()` variants *with* argument.
The argument tells the stream input how much data it needs to prefetch or the rule to attempt its match.

That is why, for example, the implementation of [`consume< Num >`](Rule-Reference.md#consume-num-) uses `if( in.size( Num ) >= Num )` instead of `if( in.size() >= Num )` to test whether the Input `in` contains at least `Num` further objects.

Similarly the `require()` and `discard()` functions are implemented for compatibility so that grammars with [`require`](Stream-Parsing.md#require-num-) and [`discard`](Stream-Parsing.md#discard) can be used on *all* inputs.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
