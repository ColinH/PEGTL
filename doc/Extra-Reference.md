# Extra Reference

The reference documentation for extra and deprecated headers.


## Contents

* [Preamble](#preamble)
* [Extras](#extras)
* [Deprecated](#deprecated)
* [Index](#index)


## Preamble

The extra headers contain classes that are experimental or otherwise not ready to be made part of the "official" public interface.

The deprecated headers contain functionality from previous versions that was not deemed useful or replaced by something better.


## Extras

The extras can be found in [`include/tao/pegtl/extra/`](../include/tao/pegtl/extra).

###### [`builders.hpp`](../include/tao/pegtl/extra/builders.hpp)

Provides [builder-style actions](Builder.md) that combine the standard builders with the numeric conversion from [`charconv.hpp`](#charconvhpp) and the string unescaping from [`unescape.hpp`](#unescapehpp).

```c++
template< auto S > struct from_chars_to;
template< typename Rule > using from_chars_for = ...;

template< auto S, template< typename... > class Unescape = unescape > using unescape_to = ...;
template< typename Rule, template< typename... > class Unescape = unescape > using unescape_for = ...;
```

The `from_chars` builders can only be used with the supported integer and floating-point types and rules from [`charconv.hpp`](#charconvhpp).
They preserve the optimized flow in which the rule parses and converts the matched input in one step.

The `unescape` builders create a `std::string` and use the supplied `Unescape` action while matching the selected rule.
The default is [`unescape`](#unescapehpp) but any compatible custom action can be supplied when the grammar uses different escape rules.

###### [`charconv.hpp`](../include/tao/pegtl/extra/charconv.hpp)

Provides string-to-number conversion based on [`std::from_chars()`](https://en.cppreference.com/w/cpp/utility/from_chars.html) from the standard [`<charconv>`](https://en.cppreference.com/w/cpp/header/charconv.html) header.
The classes defined in this header implement both the `match()` function for use as rule **and** the `apply()` function for use as action.

Integer conversion classes for decimal and hexadecimal are provided; other bases can be selected via the appropriate template parameter.

```c++
enum class overflow_mode : bool;

template< typename Integral, std::uint8_t Base, overflow_mode Over > struct from_chars_combo;

template< typename Integral, std::uint8_t Base = 10 > struct from_chars_throws;
template< typename Integral, std::uint8_t Base = 10 > struct from_chars_nothrow;

template< typename Integral > struct from_xchars_throws : from_chars_throws< Integral, 16 > {};
template< typename Integral > struct from_xchars_nothrow : from_chars_nothrow< Integral, 16 > {};

template< typename Floating, std::chars_format Format, overflow_mode Over > struct from_fchars_combo;

template< typename Floating, std::chars_format Format = std::chars_format::general > struct from_fchars_throws;
template< typename Floating, std::chars_format Format = std::chars_format::general > struct from_fchars_nothrow;
```

The `Base` parameter is passed to `std::from_chars()` as fourth function argument.
It can be anything between 2 and 36, inclusive.

The `from_fchars` classes support `float` and `double` when the standard library provides those `std::from_chars()` overloads for the deployment target.
Include `charconv.hpp` and check whether `TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS` is defined before using these classes or the floating-point builders in portable code.
Their `Format` parameter is passed to the floating-point overload of `std::from_chars()` and must be one of:

| Format | Accepted numeric form |
| ------ | --------------------- |
| `std::chars_format::general` | Decimal, with an optional `e` or `E` exponent. |
| `std::chars_format::fixed` | Decimal, without an exponent. |
| `std::chars_format::scientific` | Decimal, with a required `e` or `E` exponent. |
| `std::chars_format::hex` | Hexadecimal, with an optional `p` or `P` exponent and no `0x` prefix. |

The floating-point classes also accept infinity and NaN as supported by `std::from_chars()`.
Conversion is independent of the locale.
For example, `from_fchars_nothrow< double >` parses `"1.25e2"` as `125.0`.
With `std::chars_format::fixed`, it parses the prefix `"1.25"` and leaves `"e2"` unconsumed.

Floating-point rules pass the entire remaining input range to `std::from_chars()` and have no token-length limit.
They require a contiguous input with a no-argument `size()` function; direct use as rules on stream inputs is not supported.
Floating-point conversion actions can also be used with stream inputs because their action input provides the complete matched range.

> [!NOTE]
> Remember that `std::from_chars()` does **not** allow a leading `+` and does **not** recognize the `0x` prefix for hexadecimal numbers.

Both conversion families return `bool` from `match()` and `apply()`.
As rules, they accept arbitrary state arguments.

* If the first state argument is a mutable reference to the specified numeric type and actions are enabled, it will be assigned the conversion result.
* If the first state argument is of any other type it will be ignored -- together with any additional states.

As actions, they require the first state to be a mutable reference to the specified numeric type and assign the result only after converting the entire matched range.

The `_nothrow` suffix selects local failure, i.e. returning `false`, when `std::from_chars()` returns `std::errc::result_out_of_range`.
The `_throws` suffix selects global failure, i.e. throwing an exception derived from `tao::pegtl::parse_error_base` as described in [Errors and Exceptions](Errors-and-Exceptions.md#failure-modes).
For floating-point conversion this policy covers both overflow and underflow reported as out of range.
An unsuccessful conversion leaves the destination unchanged. Rules also leave the input unconsumed when conversion fails.

The template parameters `Integral` and `Floating` can also be `void`, which changes the behavior slightly.
In this case

* the first state *must* be a mutable reference to a supported integer type for `from_chars` and `from_xchars`, or to `float` or `double` for `from_fchars`,
* and when used as rule the `apply_mode` is ignored and the conversion result always assigned to the first state.

###### [`dispatch.hpp`](../include/tao/pegtl/extra/dispatch.hpp)

An experimental approach to attaching [actions](Actions-and-States.md) to multiple rules with less code.
Instead of specializing one action class template for every rule, a list of clauses maps rules to actions.
Rules matched by no clause use [`nothing`](Action-Reference.md#nothing-r-), while rules matched by more than one clause cause a compile-time error.

```c++
template< typename Action, typename... Rules >
struct clause1;

template< template< typename... > class Action, typename... Rules >
struct clause2;

template< typename Action, typename... Clauses >
struct default1;

template< template< typename... > class Action, typename... Clauses >
struct default2;

template< typename... Clauses >
struct dispatch;
```

The `clause1` template attaches the same concrete `Action` class to each rule in `Rules...`.
The `clause2` template attaches `Action< Rule >` to each selected rule and is useful when the action needs to depend on the matched rule type.

The `default1` and `default2` templates are enabled for rules not enabled by any of the clauses passed as their remaining template arguments.
As with the explicit clauses, `default1` uses a concrete action class while `default2` uses an action class template.

The action class template created by a dispatcher is available as `dispatch< Clauses... >::action`.
The `parse()` member function is a convenience wrapper around [`match()`](Control-and-Normal.md#match) using that action class template.

```c++
template< typename... Clauses >
struct dispatch
{
   template< typename Rule >
   using action = /* selected action, or nothing< Rule > */;

   template< typename Rule,
             apply_mode A = apply_mode::enabled,
             rewind_mode M = rewind_mode::optional,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   [[nodiscard]] static bool parse( ParseInput&&, States&&... );
};
```

Both `dispatch` and the default clauses also accept a single [`type_list`](../include/tao/pegtl/type_list.hpp) in place of a parameter pack.
See [`src/example/dispatch.cpp`](../src/example/dispatch.cpp) for a small example that dispatches different actions for selected JSON rules.

###### [`nested_exceptions.hpp`](../include/tao/pegtl/extra/nested_exceptions.hpp)

The `visit_nested` functions call the visitor once for every exception, including the first/outer one.
The visitor is called first with the inner-most exception, the one that was thrown first from furthest down/inside the call stack, and a nesting count corresponding to the total number of exceptions minus 1.
The visitor is called last with the outer-most exception, the one that was caught in the try-catch that is calling `visit_nested()`, and a nesting count of `0`.

For example, when an exception `E` is caught, and `E` has exception `F` as nested exception, then `visit_nested( E, V )` for some visitor `V` will first call `V( F, 1 )` and then `V( E, 0 )`.

Exceptions in an inheritance hierarchy must be listed from more general to more specific in the template parameter list.
For example `visit_nested< std::exception, std::runtime_error >()` and not vice versa.

```c++
template< typename... Exceptions, typename Visitor >
void visit_nested( Visitor&& );

template< typename... Exceptions, typename Visitor >
void visit_nested( const std::exception_ptr&, Visitor&& );

template< typename... Exceptions, typename Exception, typename Visitor >
void visit_nested( const Exception&, Visitor&& );
```

The following convenience functions call `visit_nested()` and produce a flattened vector derived from the nested exceptions.
The `flatten_type()` and `flatten_base()` functions return type sliced copies of the exceptions; `flatten_what()` calls `what()` on the `std::exception`s and returns a vector of strings.

```c++
template< typename Exception >
[[nodiscard]] std::vector< Exception > flatten_type( const std::exception_ptr& = std::current_exception() );
template< typename Exception >
[[nodiscard]] std::vector< Exception > flatten_type( const Exception& );

[[nodiscard]] std::vector< parse_error_base > flatten_base( const std::exception_ptr& = std::current_exception() )
[[nodiscard]] std::vector< parse_error_base > flatten_base( const parse_error_base& );

[[nodiscard]] std::vector< std::string > flatten_what( const std::exception_ptr& = std::current_exception() )
[[nodiscard]] std::vector< std::string > flatten_what( const std::exception& );
```

###### [`parse_tree.hpp`](../include/tao/pegtl/extra/parse_tree.hpp)
###### [`parse_tree_to_dot.hpp`](../include/tao/pegtl/extra/parse_tree_to_dot.hpp)

The [parse tree has its own dedicated page](Parse-Tree.md).

###### [`overflow_mode.hpp`](../include/tao/pegtl/extra/overflow_mode.hpp)

Defines `enum class overflow_mode` with values `local_failure` and `global_failure`.
This enum is used by [`charconv.hpp`](#charconvhpp) to choose whether an out-of-range numeric conversion causes a local failure or a global failure.

###### [`partial_apply1.hpp`](../include/tao/pegtl/extra/partial_apply1.hpp)

An experimental rule that calls actions in an unusual way.

* Based on [`partial< R... >`](Rule-Reference.md#partial-r-) but with major differences:
* First, `R` must contain at least *two* rules.
* Second, only succeeds when (at least) the first sub-rule succeeds, i.e.
* the "partial" matching behavior only starts at the second rule.
* Equivalent to `seq< R1, partial< R2, ... > >` when `R` is `R1`, `R2`, ...
* Third, the `R...` are matched with [actions](Actions-and-States.md) disabled, however:
* Fourth, on success exactly one action is applied
  - for the last rule in `R...` that matched, and
  - with an action input spanning from the start of `partial_apply1` to the current input position.
* Fifth, if actions are enabled for `partial_apply1` **and**
  - `Action< T >` is not exactly `tao::pegtl::nothing< T >` **then**
  - `Action< T >` must implement `apply()` or `apply0()` for all `T` in `R...`
  - (assuming that `Action` is the current action in the parsing run).
* Meta data and implementation mapping:
  - `partial_apply1< R... >::rule_t` is `internal::partial_apply1< R... >`
  - `partial_apply1< R... >::subs_t` is `type_list< R... >`

###### [`raw_string.hpp`](../include/tao/pegtl/extra/raw_string.hpp)

Rules for parsing [Lua](https://lua.org)-style *long string literals*, also called *raw literals* because they do not support any escape sequences.

The following description of these kinds of string literals is copied from the [Lua](https://lua.org) documentation.

* An "opening long bracket of level n" is defined as an opening square bracket followed by *n* equal signs followed by another opening square bracket. So, an opening long bracket of level 0 is written as `[[`, an opening long bracket of level 1 is written as `[=[`, and so on.
* A "closing long bracket" is defined similarly; for instance, a closing long bracket of level 4 is written as `]====]`.
* A "long literal" starts with an opening long bracket of any level and ends at the first closing long bracket of the same level. It can contain any text except a closing bracket of the same level.
* Literals in this bracketed form can run for several lines, do not interpret any escape sequences, and ignore long brackets of any other level.
* For convenience, when the opening long bracket is eagerly followed by a newline, the newline is not included in the string.

Unlike Lua's long literal the `raw_string` rule is customizable to use other characters than `[`, `=` and `]` for matching the opening and closing long brackets.
Also note that Lua introduced newline-specific replacements in Lua 5.2, which are not supported at the grammar level by the `raw_string` rule.

The `raw_string` rule has a sub-type called `content` that actions should usually be bound to instead of binding to `raw_string` itself.
Binding to `content` will call the action with the matched string *excluding* the opening and closing long brackets.

```c++
template< char Open, char Marker, char Close, typename... Contents >
struct raw_string
   // base is implementation detail
{
   // struct content; -- inherited
};

template< char Open, char Marker, char Close >
struct raw_string< Open, Marker, Close >
   // base is implementation detail
{
   // struct content; -- inherited
};
```

###### [`record.hpp`](../include/tao/pegtl/extra/record.hpp)

Builds on [`dispatch.hpp`](#dispatchhpp) to build a linear record of a parsing run.
For each successful match of a selected rule the vector contains an entry with a kind of `view_input` to represent the matched input, and the name of the rule that matched.
Successful sub-rules of a failed rule will not show up in the record.

```c++
template< typename ParseInput >
using record_input = input_with_offset< internal::text_view_alias< typename ParseInput::eol_rule, typename ParseInput::data_t, void, void > >;

template< typename ParseInput >
struct record_value
{
   using data_t = typename ParseInput::data_t;
   using position_t = typename ParseInput::error_position_t;

   record_value( const position_t&, const data_t*, const std::size_t, const std::string_view );

   record_input< ParseInput > input;
   std::string_view rule;
};

template< typename ParseInput >
using record_vector = std::vector< record_value< ParseInput > >;

template< typename... Rules >
struct record
{
   using clause = // clauses to produce the record for Rules...

   template< typename Rule >
   using action = // clauses converted to an action

   template< typename Rule,
             template< typename... > class Control = normal,
             typename ParseInput >
   [[nodiscard]] static auto parse( ParseInput&& )
   {
      // parse with action and return a record_vector< ParseInput >
   }
};
```

###### [`type_to_string.hpp`](../include/tao/pegtl/extra/type_to_string.hpp)

Functions to convert compile-time strings or sequences of `char` into a `std::string` or `std::string_view`.

```c++
template< typename T >
[[nodiscard]] std::string type_to_string();

template< char... Cs >
[[nodiscard]] std::string type_to_string();

template< typename T >
[[nodiscard]] constexpr std::string_view type_to_string_view() noexcept;

template< char... Cs >
[[nodiscard]] constexpr std::string_view type_to_string_view() noexcept;
```

The overloads that take a single type support the ASCII string-literal rules, their character-pack forms, and `raise_message` and `raise_message_chars`.
For example `type_to_string< string< "abc" > >()` returns `"abc"`.
For `astring` and `aistring` they return the complete string, including the optional suffix.
Embedded null bytes are preserved.

The type overloads also accept other class templates with `char...` parameters, optionally preceded by a `std::size_t` parameter.

###### [`unescape.hpp`](../include/tao/pegtl/extra/unescape.hpp)

A slightly refactored version of the previous unescape header which is now moved to `deprecated/unescape.hpp`.

It defines individual [action](Actions-and-States.md) classes for common unescape operations as well as an action class template that attaches these actions to corresponding grammar rules.

All actions output to their first state (second argument) which must be a mutable `std::string`.

```c++
struct all_of_these_actions
{
   template< typename ActionInput >
   static void apply( const ActionInput& in, std::string& out );
};
```

`unescape_append` is for non-escaped characters that need to just be copied from the input unchanged by calling `out.append( in.string_view() );`

`unescape_char<>` is for single character escape sequences like `\n`.
It takes any number of template parameters of type `char` and has to be attached to a rule of type `(ascii::)one<>` with the same number of template parameters.

During unescape the n-th template parameter of the `one` is mapped to the n-th template parameter of `unescape_char`.

`unescape_byte` is for bytes represented by two hex digits and must be attached to a rule that matches exactly that.

`unescape_unicode<>` is for hexadecimal Unicode code points, excluding surrogates.
The template parameter of type `std::size_t` must match the fixed (!) number of hex digits the rule it is attached to matches, usually 4 or 8.

`json_unescape_unicode` is for sequences of hexadecimal UTF-16 code points with 4 hex digits each.
It unescapes each individual code point while checking for and merging consecutive surrogate pairs as required by RFC 8259.

> [!WARNING]
> The actions do **not** double-check that the matched portion of the input they are called with has the expected size and data!

The action template `unescape<>` is set up with the usual default

```c++
template< typename Rule > unescape< Rule > : nothing< Rule > {};
```

and some useful specializations for some generic rules, and some specific rules defined in [`include/tao/pegtl/example/escaped.hpp`](Example-Reference.md#escapedhpp), that might be part of a rule for strings with escape sequences.
Please check out `include/tao/pegtl/extra/unescape.hpp` to see which individual unescape action is attached to which rule by `unescape<>`.


## Deprecated

Deprecated headers can still be found in [`include/tao/pegtl/deprecated/`](../include/tao/pegtl/deprecated).

###### [`alphabet.hpp`](../include/tao/pegtl/deprecated/alphabet.hpp)

Constants and rules for alphabetic ASCII characters.

###### [`if_then.hpp`](../include/tao/pegtl/deprecated/if_then.hpp)

Experimental combinators for arbitrary if-then-else rule cascades.

###### [`integer.hpp`](../include/tao/pegtl/deprecated/integer.hpp)

Rules and actions for integer parsing, replaced by [`charconv.hpp`](#charconvhpp).

###### [`rep_one_min_max.hpp`](../include/tao/pegtl/deprecated/rep_one_min_max.hpp)

An optimized version of `tao::pegtl::rep_min_max< tao::pegtl::ascii::one< ... > >`.

###### [`rep_string.hpp`](../include/tao/pegtl/deprecated/rep_string.hpp)

An optimized version of `tao::pegtl::rep< Num, tao::pegtl::ascii::string< Text > >`.

###### [`unescape.hpp`](../include/tao/pegtl/deprecated/unescape.hpp)

An older version of [`include/tao/pegtl/extra/unescape.hpp`](#unescapehpp).


## Index

* [`alphabet.hpp`](#alphabethpp) <sup>[(deprecated)](#deprecated)</sup>
* [`builders.hpp`](#buildershpp) <sup>[(extra)](#extras)</sup>
* [`charconv.hpp`](#charconvhpp) <sup>[(extra)](#extras)</sup>
* [`dispatch.hpp`](#dispatchhpp) <sup>[(extra)](#extras)</sup>
* [`if_then.hpp`](#if_thenhpp) <sup>[(deprecated)](#deprecated)</sup>
* [`integer.hpp`](#integerhpp) <sup>[(deprecated)](#deprecated)</sup>
* [`nested_exceptions.hpp`](#nested_exceptionshpp) <sup>[(extra)](#extras)</sup>
* [`overflow_mode.hpp`](#overflow_modehpp) <sup>[(extra)](#extras)</sup>
* [`parse_tree.hpp`](#parse_treehpp) <sup>[(extra)](#extras)</sup>
* [`parse_tree_to_dot.hpp`](#parse_tree_to_dothpp) <sup>[(extra)](#extras)</sup>
* [`partial_apply1.hpp`](#partial_apply1hpp) <sup>[(extra)](#extras)</sup>
* [`raw_string.hpp`](#raw_stringhpp) <sup>[(extra)](#extras)</sup>
* [`record.hpp`](#recordhpp) <sup>[(extra)](#extras)</sup>
* [`rep_one_min_max.hpp`](#rep_one_min_maxhpp) <sup>[(deprecated)](#deprecated)</sup>
* [`rep_string.hpp`](#rep_stringhpp) <sup>[(deprecated)](#deprecated)</sup>
* [`type_to_string.hpp`](#type_to_stringhpp) <sup>[(extra)](#extras)</sup>
* [`unescape.hpp`](#unescapehpp) <sup>[(extra)](#extras)</sup>
* [`unescape.hpp`](#unescapehpp-1) <sup>[(deprecated)](#deprecated)</sup>


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
