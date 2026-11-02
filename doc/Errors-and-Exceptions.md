# Errors and Exceptions

Parsing rule and run results are Boolean values or exceptions.
This page describes how they interact with [backtracking](https://en.wikipedia.org/wiki/Backtracking), how the PEGTL represents [parse errors](#parse-errors), and how grammars and controls can produce useful diagnostics.


## Contents

* [Failure Modes](#failure-modes)
* [Parse Errors](#parse-errors)
* [Catching Errors](#catching-errors)
* [Local to Global Failure](#local-to-global-failure)
  * [Must Rules](#must-rules)
  * [Raise Rules](#raise-rules)
  * [Best Practices](#best-practices)
* [Custom Error Messages](#custom-error-messages)
  * [Rule Error Message](#rule-error-message)
  * [Raise Message](#raise-message)
  * [must_if Control](#must_if-control)
  * [Custom Control](#custom-control)
* [Global to Local Failure](#global-to-local-failure)
* [Nested Exceptions](#nested-exceptions)
* [Error Positions](#error-positions)
* [No Exception Support](#no-exception-support)


## Failure Modes

A [`match()` function](Rules-and-Grammars.md#match-function), and therefore a call to one of the [`parse()` functions](Inputs-and-Parsing.md#parse-function), has three possible outcomes.

* A return value of `true` indicates a successful match. In this case a rule is allowed to consume some input.
* A return value of `false` indicates a *local failure*. In this case a rule is only allowed to consume input when the `rewind_mode` is `optional`.
* An exception indicates a *global failure* which usually aborts a parsing run. In this case a rule is also allowed to consume input.

In the case of local failure what happens next depends on the grammar.
For example when a sub-rule of [`seq`](Rule-Reference.md#seq-r-) returns `false` the sequence will also return `false`, when a sub-rule of a [`sor`](Rule-Reference.md#sor-r-) returns `false` the next rule will be attempted to match at the same position (provided the failed rule was not the last one) which we call *backtracking*, and when a sub-rule of an [`opt`](Rule-Reference.md#opt-r-) returns `false` the optional will still succeed.

Exceptions indicating a global failure can stem from multiple sources.
The PEGTL rules that create global failures throw exceptions derived from `tao::pegtl::parse_error_base`, usually via the current [control](Control-and-Normal.md) class' `raise()` function.
Inputs can throw other exceptions, for example `std::filesystem_error` when opening a file or stream-related exceptions while reading.
Actions and custom rules can throw any exception type.

The "do not consume" requirement in the local failure case **must** be strictly followed, though the library attempts to minimize the use of `rewind_mode::optional` where it is not necessary.
For example, some callers already have a rewind guard around a larger expression, and top-level parses usually do not need the input restored when the whole parse returns `false`.
The details are controlled by the [`rewind_mode`](Rules-and-Grammars.md#rewind-guard) used by the parser and by rule implementations.

The important practical guideline is simple: A grammar author uses local failure for normal PEG alternatives, and turns a failure into a global failure for non-recoverable conditions.


## Parse Errors

The default global parse error is `tao::pegtl::parse_error< Position >`, which derives from `tao::pegtl::parse_error_base`.

The header `include/tao/pegtl/parse_error_base.hpp` contains the non-position-dependent base class.
The header `include/tao/pegtl/parse_error.hpp` contains the position-dependent derived class and helper functions.

```c++
namespace tao::pegtl
{
   class parse_error_base
      : public std::runtime_error
   {
   public:
      [[nodiscard]] std::string_view message() const noexcept;
      [[nodiscard]] std::string_view position_string() const noexcept;
   };

   template< typename Position >
   class parse_error
      : public parse_error_base
   {
   public:
      using position_t = Position;

      parse_error( const std::string& msg, Position&& pos );
      parse_error( const std::string& msg, const Position& pos );

      [[nodiscard]] const position_t& position_object() const noexcept;
   };

   template< typename Position >
   parse_error( const std::string&, Position ) -> parse_error< std::decay_t< Position > >;

   template< typename Object >
   [[noreturn]] void throw_parse_error( const std::string& msg, const Object& obj );

   template< typename Object >
   [[noreturn]] void throw_parse_error_with_nested( const std::string& msg, const Object& obj );
}
```

The `message()` function returns the message that was supplied when the exception was created.
The `position_string()` function returns the position formatted as a string.
The inherited `what()` function returns both, formatted as `"position: message"`.

The `position_object()` function is only available on the concrete `parse_error< Position >` type.
It returns the stored position object.

The helper function `throw_parse_error()` accepts either an input object or an already existing position object.
It extracts the current position when given an input, creates the appropriate `parse_error< Position >`, and throws it.
The helper function `throw_parse_error_with_nested()` similarly calls `std::throw_with_nested()`.

Most [input classes](Input-Reference.md) define a `parse_error_t` alias for their concrete parse error type.
For example, for a `text_file_input<>` this is `parse_error< text_file_input<>::error_position_t >`, with text position and source information.


## Catching Errors

When a catch block wants to print a source line, it needs access to the input object.
This is why many examples construct the input first, then put the parsing run in a nested `try` block.

```c++
try {
   using input_t = tao::pegtl::text_file_input< tao::pegtl::scan::lf_crlf >;
   input_t in( filename );

   try {
      tao::pegtl::parse< grammar >( in );
   }
   catch( const input_t::parse_error_t& e ) {
      const auto& p = e.position_object();
      std::cerr << e.what() << '\n'
                << in.line_view_at( p ) << '\n'
                << std::setw( int( p.column ) ) << '^' << std::endl;
      return 1;
   }
}
catch( const tao::pegtl::parse_error_base& e ) {
   std::cerr << e.what() << std::endl;
   return 1;
}
catch( const std::exception& e ) {
   std::cerr << e.what() << std::endl;
   return 1;
}
```

The first catch block catches exactly the parse errors whose position type is used by `input_t`, which makes `position_object()` available with the correct static type.
The second catch block catches other PEGTL parse errors without assuming a particular position type.
The final catch block catches other standard exceptions, for example file and stream errors.

The source files `src/example/json_parse.cpp`, `src/example/json_classes.cpp`, `src/example/proto3_parse.cpp`, and `src/example/abnf2pegtl.cpp` show variations of this pattern.


## Local to Global Failure

A local failure returned by a parsing rule is not necessarily propagated to the top.
For example, when a rule is a sub-rule of `not_at<>`, `opt<>` or `star<>`, a local failure of the sub-rule can still allow the containing rule to succeed.
When a sub-rule of an `sor<>` fails locally, the `sor<>` can try the next alternative.

This is exactly what makes PEG grammars useful, but it also means that a grammar should mark the points where backtracking is no longer desirable.

### Must Rules

The primary way to turn local failure into global failure is the [`must<>`](Rule-Reference.md#must-r-) family of rules.

`must< R... >` is similar to `seq< R... >`, but it converts local failure of the direct sub-rules into a call to the control class' `raise()` function.
With the default `normal` control this throws a `parse_error< ParseInput::error_position_t >`, where `ParseInput` is the current input type.

The related rules include

* `if_must< R, S... >`,
* `if_must_else< R, S, T >`,
* `opt_must< R, S... >`,
* `star_must< R, S... >`,
* `list_must< R, S >`, and
* `list_must< R, S, P >`.

For example, consider a simplified string escape grammar.

```c++
using namespace tao::pegtl;

struct escaped : seq< one< '\\' >, one< 'n', 'r', 't' > > {};
struct content : star< sor< escaped, not_one< '\\', '"' > > > {};
struct literal : seq< one< '"' >, content, one< '"' > > {};
```

If `escaped` sees a backslash followed by some other character, it fails locally.
The surrounding `sor<>` then tries `not_one< '\\', '"' >` at the same position.
That backtracking is useful when the next character is not a backslash, but it is not useful after the backslash has already matched.

The rule can be written as follows.

```c++
using namespace tao::pegtl;

struct escchar : one< 'n', 'r', 't' > {};
struct escaped : if_must< one< '\\' >, escchar > {};
struct content : star< sor< escaped, not_one< '"' > > > {};
struct literal : seq< one< '"' >, content, one< '"' > > {};
```

Now `escaped` returns local failure only when the next input character is not a backslash.
If a backslash is present, the escape character must match, otherwise a parse error is thrown.

### Raise Rules

The rule [`raise< T >`](Rule-Reference.md#raise-t-) unconditionally generates a global failure by calling `Control< T >::raise()`.
The template argument `T` can be any type; it does not have to be a parsing rule.

The rule [`raise_message< Text >`](Rule-Reference.md#raise_message-text-) is a convenience rule that raises with the custom static message given by a string literal.

```c++
using namespace tao::pegtl;

struct value
   : sor< number, string, raise_message< "expected value" > > {};
```

This is useful as the final alternative of an `sor<>`, or in a helper rule whose only purpose is to report a specific error.

### Best Practices

A useful grammar usually contains fewer error points than rules.
Place global failures where the parser has enough context to know what was expected.

Good places are often after a delimiter, keyword, opening bracket, separator, or other prefix that commits the grammar to a specific construct.
For example, once a string literal has matched its opening quote, an unterminated string is better reported as a parse error than as a failed alternative somewhere higher in the grammar.

Giving important rules dedicated names improves both default and custom error messages.
Prefer

```c++
struct escchar : one< 'n', 'r', 't' > {};
struct escaped : if_must< one< '\\' >, escchar > {};
```

over putting a large anonymous template instantiation directly inside `must<>`.
Named rules also avoid accidentally attaching the same action or error message to the same structural rule in several places.

At the top level, `must< grammar, eof >` is sometimes convenient when a program wants the parsing run to either succeed or throw, instead of returning `false`.
When the top-level rule is already a complete grammar, `seq< grammar, eof >` plus a check of the `parse()` return value is equally valid.


## Custom Error Messages

By default, `normal< Rule >::raise()` throws a parse error with a message of the form

```text
parse error matching <demangled rule name>
```

There are several ways to replace this with better messages.

### Rule Error Message

The default control checks whether the rule passed to `raise()` has a static `error_message` member.
When it exists, that string is used instead of the demangled rule name.

```c++
struct close_quote
   : tao::pegtl::one< '"' >
{
   static constexpr const char* error_message = "unterminated string";
};

struct literal
   : tao::pegtl::if_must< tao::pegtl::one< '"' >,
                          tao::pegtl::star< tao::pegtl::not_one< '"' > >,
                          close_quote > {};
```

This is the lightest-weight approach when the message belongs naturally to a named rule.
The test `src/test/error_message_3.cpp` shows the minimal form.

### Raise Message

For one-off error points, use `raise_message< "message" >`.

```c++
struct grammar
   : tao::pegtl::sor< option_a,
                      option_b,
                      tao::pegtl::raise_message< "expected option" > > {};
```

The test `src/test/error_message_2.cpp` shows this pattern.
The message is supplied as a string-literal template argument.

### must_if Control

For larger grammars it is often preferable to collect messages in one place and install them through a control adapter.
The adapter `must_if_n` is defined in `include/tao/pegtl/control/must_if.hpp`.

```c++
template< typename Errors,
          bool RequireMessage = true,
          template< typename... > class Control = normal >
struct must_if_n
{
   template< typename Rule >
   using type = /* implementation */;
};
```

The `Errors` type must provide a variable template `message< Rule >` whose value is either a `const char*` or `nullptr`.
One common setup is to use a variable template for the actual messages and wrap it in a small type.

```c++
template< typename >
inline constexpr const char* error_message = nullptr;

template<>
inline constexpr auto error_message< tao::pegtl::json::text > = "no valid JSON";

template<>
inline constexpr auto error_message< tao::pegtl::json::end_array > = "incomplete array, expected ']'";

template<>
inline constexpr auto error_message< tao::pegtl::eof > = "unexpected character after JSON value";

struct errors
{
   template< typename Rule >
   static constexpr auto message = error_message< Rule >;
};

template< typename Rule >
using control = tao::pegtl::must_if_n< errors >::type< Rule >;
```

The parse call then passes this control as the third template parameter.

```c++
tao::pegtl::parse< grammar, tao::pegtl::nothing, control >( in );
```

When `Errors::message< R >` is not `nullptr`, `must_if_n` uses it for calls to `raise()` and `raise_nested()` for `R`.
By default it also turns local failure of `R` into a global failure.
This gives a non-intrusive way to add error points without changing the grammar itself.

This default is useful, but it should be used deliberately.
If the same rule occurs in contexts where local failure should remain local, override `raise_on_failure`.

```c++
struct errors
{
   template< typename Rule >
   static constexpr bool raise_on_failure = false;

   template< typename Rule >
   static constexpr auto message = error_message< Rule >;
};
```

With the default `RequireMessage = true`, a call to `raise()` for a rule without either an `Errors::message< Rule >` entry or a rule-local `error_message` member produces a compile-time error.
Set the second template argument to `false` to allow fallback to the adapted control.

```c++
template< typename Rule >
using control = tao::pegtl::must_if_n< errors, false >::type< Rule >;
```

The third template argument adapts another control instead of `normal`.
This is useful when custom error messages should be combined with another control adapter.

The examples `src/example/json_errors.hpp` and `src/example/abnf_errors.hpp` show the intended larger-grammar pattern.
The test `src/test/error_message_1.cpp` shows the minimal form.

### Custom Control

For complete control over parse errors, provide a [custom control class](Control-and-Normal.md#control-interface) with `raise()` and optionally `raise_nested()`.

```c++
template< typename Rule >
struct control
   : tao::pegtl::normal< Rule >
{
   template< typename ParseInput, typename... States >
   [[noreturn]] static void raise( const ParseInput& in, States&&... st )
   {
      std::cerr << in.current_position()
                << ": parse error matching "
                << tao::pegtl::demangle< Rule >() << std::endl;

      tao::pegtl::normal< Rule >::raise( in, st... );
   }
};
```

The example `src/example/recover.cpp` uses this technique to print diagnostic information before delegating to `normal`.

Custom rules that need to raise a PEGTL parse error should usually call the active control's `raise()` function when they have the complex `match()` interface.
This preserves user-selected error behavior.
When no control is available, `throw_parse_error()` can be used directly.


## Global to Local Failure

The `try_catch_return_false` family of rules converts selected exceptions into local failure.
This can be useful for recovery-oriented grammars where a failed sub-parse should be skipped and the outer grammar should continue.

The available variants are

* `try_catch_return_false< R... >` for `parse_error_base`,
* `try_catch_any_return_false< R... >` for any exception,
* `try_catch_std_return_false< R... >` for `std::exception`, and
* `try_catch_type_return_false< E, R... >` for a chosen exception type `E`.

If an exception does not match the variant's catch type, it propagates normally.

The example `src/example/recover.cpp` uses `try_catch_return_false< must< R >, T >` together with an alternative that skips to a terminator.
This is a useful pattern for tools that need to report multiple errors from one input.

Use this sparingly in ordinary parsers.
Converting global failures back to local failures can make diagnostics harder to reason about when it is applied too broadly.


## Nested Exceptions

Nested exceptions are used to keep multiple positions for one error.
The most common use case is [nested parsing](Inputs-and-Parsing.md#nested-parsing), for example a file that includes another file.

The function [`parse_nested()`](Inputs-and-Parsing.md#nested-parsing) catches `std::exception` from the inner parsing run and calls `Control< Rule >::raise_nested()` with the ambient outer input or position.
The default `normal` control throws a new `parse_error< Position >` for the outer input or position, with the caught exception as nested exception.

The `try_catch_raise_nested` family performs a similar transformation inside a grammar.
The available variants are

* `try_catch_raise_nested< R... >` for `parse_error_base`,
* `try_catch_any_raise_nested< R... >` for any exception,
* `try_catch_std_raise_nested< R... >` for `std::exception`, and
* `try_catch_type_raise_nested< E, R... >` for a chosen exception type `E`.

The header `include/tao/pegtl/extra/nested_exceptions.hpp` provides helpers to inspect nested exception chains.
It requires exception and RTTI support.

```c++
#include <tao/pegtl/extra/nested_exceptions.hpp>

try {
   tao::pegtl::parse_nested< included_grammar >( outer_input, included_input );
}
catch( ... ) {
   const auto errors = tao::pegtl::flatten_base();
   for( const auto& e : errors ) {
      std::cerr << e.what() << std::endl;
   }
}
```

See `src/test/rule_try_catch_raise_nested.cpp`, `src/test/extra_nested_exceptions.cpp`, and [Extra Reference](Extra-Reference.md#nested_exceptionshpp) for the details.


## Error Positions

When reporting an error, it is often useful to print the complete source line and a marker at the position within that line.
Inputs that track line starts provide `line_view_at()`, which returns a `std::string_view` for the line containing a position.

```c++
try {
   tao::pegtl::parse< grammar >( in );
}
catch( const decltype( in )::parse_error_t& e ) {
   const auto& p = e.position_object();
   std::cerr << e.what() << '\n'
             << in.line_view_at( p ) << '\n'
             << std::setw( int( p.column ) ) << '^' << std::endl;
}
catch( const tao::pegtl::parse_error_base& e ) {
   std::cerr << e.what() << std::endl;
}
```

For text positions, `p.line` and `p.column` are one-based and `p.count` is the zero-based count from the start of the input.
When printing a caret under `line_view_at( p )`, use `p.column`, not `p.count`.

The caret position is visually reliable for graphical ASCII without tabs.
UTF-8, combining characters, tabs, terminal width rules, and non-text inputs can require application-specific formatting.

Some inputs use count or pointer positions instead of text positions, and some text inputs can track positions lazily.
The exact position type is documented in [Inputs and Parsing](Inputs-and-Parsing.md#position-classes).


## No Exception Support

When compiling without exception support, the exceptional rules and the [parse error](#parse-errors) classes are not available.
Headers that require exceptions contain preprocessor checks.

Programs that support both modes usually put exception-based diagnostics under `#if defined( __cpp_exceptions )` and otherwise check the boolean return value of `parse()`.
The example `src/example/json_errors.hpp` falls back to `normal` control without exceptions, and `src/example/json_parse.cpp` prints a simple error when `parse()` returns `false`.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
