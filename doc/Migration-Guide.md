# Migration Guide

Notes on incompatible changes that need to be taken into account when updating to a new major version.


## Version 5.0.0

* The build system needs to be configured for C++23 or later and [needs a modern compiler](Install-Guide.md#requirements).
* Inputs passed to `parse()` and the inner input passed to `parse_nested()` must not be `const`.

### String Rules

The ASCII string rules and `raise_message` now take string literals directly.
Replace the old character-pack arguments and string-literal macros as follows:

| Before | PEGTL 5.0 |
| --- | --- |
| `string< 'f', 'o', 'o' >` or `TAO_PEGTL_STRING( "foo" )` | `string< "foo" >` |
| `istring< 'f', 'o', 'o' >` or `TAO_PEGTL_ISTRING( "foo" )` | `istring< "foo" >` |
| `keyword< 'f', 'o', 'o' >` or `TAO_PEGTL_KEYWORD( "foo" )` | `keyword< "foo" >` |
| `astring< 1, 'f', 'o', 'o' >` | `astring< 1, "foo" >` |
| `aistring< 1, 'f', 'o', 'o' >` | `aistring< 1, "foo" >` |
| `raise_message< 'f', 'o', 'o' >` or `TAO_PEGTL_RAISE_MESSAGE( "foo" )` | `raise_message< "foo" >` |

Code that produces character packs can use the [compatibility rules](Rule-Reference.md#character-packs) `string_chars`, `istring_chars`, `keyword_chars`, `astring_chars`, `aistring_chars` and `raise_message_chars`.
The character-pack and string-literal forms are distinct types; update action and control specializations to match the form used by the grammar.
The old macros and `<tao/pegtl/pegtl_string.hpp>` were removed.
The Unicode, binary and member string rules retain their value-pack interfaces.


## Version 4.0.0

Version 4.0 contains a large refactoring of the public interface.
Most grammars should continue to look familiar, however almost all code that constructs inputs, catches parse errors, or includes former `contrib` headers needs attention.
Please check the full list of added, renamed and changed rules, actions, controls, inputs, examples etc. in the [changelog](Changelog.md#version-400).

### Build Requirements

PEGTL 4.0 still requires C++17, however the supported compiler baseline was raised.
See the [install guide](Install-Guide.md#requirements) for the current list of supported compilers.

With GCC 16.1, `tao::pegtl::demangle()` falls back to `typeid( T ).name()` due to [bug #91155](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91155).

### Input Classes

The old input hierarchy was replaced.
The most important decision when migrating is whether client code needs line and column numbers in error positions.
In PEGTL 3.x the common inputs reported a `position` with `source`, `byte`, `line` and `column`.
In PEGTL 4.x the plain inputs usually report a count-only position, or a pointer-only position for [`base_input`](Input-Reference.md#base-input); use the `text_` inputs when the old line-and-column behavior is required.

| PEGTL 3.x | PEGTL 4.x with line/column | PEGTL 4.x count-only |
| --- | --- | --- |
| `memory_input` over existing memory | `text_view_input` | `view_input` |
| `string_input` that owns a copy | `text_copy_input` | `copy_input` |
| `file_input` | `text_file_input` | `file_input` |
| `read_input` | `text_read_input` | `read_input` |
| `mmap_input` | `text_mmap_input` | `mmap_input` |
| `argv_input` | `text_view_input( std::string_view( argv[n] ) )` | `argv_input` |
| `istream_input`, `cstream_input`, `buffer_input` | `alloc_text_istream_input`, `array_text_cstream_input`, etc. from `<tao/pegtl/stream.hpp>` | `alloc_istream_input`, `array_cstream_input`, etc. from `<tao/pegtl/stream.hpp>` |

The `tracking_mode` template parameter was removed.
Line tracking is now selected by the input class and the end-of-line rule:

* plain inputs such as `view_input` and `file_input` do not store line and column numbers;
* `text_` inputs store line and column numbers;
* `ascii::scan::...` and `utf8::scan::...` end-of-line rules preserve the old eager scan behavior, where line numbers are updated even when a rule other than `eol` consumes a newline;
* normal end-of-line rules such as `ascii::lf` update line numbers only when the grammar matches `eol` or `eolf`;
* `ascii::lazy::...` and `utf8::lazy::...` calculate line and column numbers on demand.

The old default was effectively `tracking_mode::eager` with `eol::lf_crlf`.
For the closest 4.x replacement use an explicit `ascii::scan::lf_crlf`, for example `text_file_input< ascii::scan::lf_crlf >`.
The new default [`default_eol`](../include/tao/pegtl/default_eol.hpp) is platform-dependent: `ascii::scan::lf` on POSIX systems and `ascii::scan::lf_crlf` on Windows.

The old source argument is no longer mandatory for memory inputs.
When a source should be stored in error positions, pass it as the first constructor argument and use source template parameters when CTAD is not enough, for example `text_view_input< ascii::scan::lf_crlf, char, std::string, std::string >`.
For file inputs the source is always the filesystem path.

The depth counter is no longer part of every input.
Code that uses depth limiting must include [`<tao/pegtl/input_with_depth.hpp>`](../include/tao/pegtl/input_with_depth.hpp) and wrap the input type with [`input_with_depth< Input >`](Input-Reference.md#input-with-depth).

Offsets are no longer passed to memory input constructors or `restart()`.
Code that needs positions relative to a non-zero starting point should include [`<tao/pegtl/input_with_offset.hpp>`](../include/tao/pegtl/input_with_offset.hpp) and wrap the input type with [`input_with_offset< Input >`](Input-Reference.md#input-with-offset), passing a `count_position` or `text_position` offset as the first constructor argument.

### Positions

The single `position` class was replaced by several position types.
Use `text_position` for line/column/count positions, `count_position` for count-only positions, `pointer_position` for raw pointer positions, and `position_with_source< Source, Position >` when a source object is present.

Code that reads `position::byte` must use `count`.
Code that expects `source` directly on every position must account for positions without sources, or use inputs whose `error_position_t` is a `position_with_source`.
When working generically, prefer `Input::error_position_t` instead of spelling out a concrete position type.

Several input member functions were renamed or split:

* `in.position()` is now `in.current_position()`.
* `in.source()` is now `in.direct_source()` for inputs that have a source.
* `in.begin()` is now `in.start()` when the start of the input is meant; use `in.current()` for the current parse position.
* `in.end_of_line( p )` is now `in.end_of_line_or_file( p )`.
* `in.line_at( p )` is now `in.line_view_at( p )`.

The `action_t` alias was removed from [input classes](Input-Reference.md).
Actions should either template the first argument of `apply()`, as already recommended for PEGTL 3.x, or use [`tao::pegtl::action_input< ParseInput >`](Actions-and-States.md#action-input).
Inside actions, use `in.current_position()` instead of the old `in.position()`.

### Exceptions

The [`parse_error`](Errors-and-Exceptions.md#parse-errors) is now a class template over the stored position type, and all parse errors derive from the non-template base class `parse_error_base`.
Code that used to catch `const parse_error&` can often just catch [`const parse_error_base&`](Errors-and-Exceptions.md#parse-errors).

```c++
try {
   tao::pegtl::text_file_input in( file );
   tao::pegtl::parse< grammar >( in );
}
catch( const tao::pegtl::parse_error_base& e ) {
   std::cerr << e.what() << '\n';
}
```

A `parse_error` now contains exactly one position object.
Use `e.position_object()` when catching a concrete `parse_error< Position >`, or use `e.position_string()` when catching `parse_error_base`.
The old `positions()` vector and `add_position()` mechanism were removed.

Nested parsing no longer appends positions to a single [parse error](Errors-and-Exceptions.md#parse-errors).
[`parse_nested()`](Inputs-and-Parsing.md#nested-parsing) now throws a nested exception that wraps the inner exception with the outer position.
Include [`<tao/pegtl/extra/nested_exceptions.hpp>`](Extra-Reference.md#nested_exceptionshpp) and use `visit_nested()`, `flatten_base()`, `flatten_type()` or `flatten_what()` to inspect such chains.
The helper header requires exceptions and [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information).

[`parse_error.hpp`](../include/tao/pegtl/parse_error.hpp) and [`parse_error_base.hpp`](../include/tao/pegtl/parse_error_base.hpp) require exceptions.
When compiling without exceptions, code must not include or use these facilities.
The umbrella header [`<tao/pegtl.hpp>`](../include/tao/pegtl.hpp) also includes [`parse_nested.hpp`](../include/tao/pegtl/parse_nested.hpp) only when exceptions are enabled.
Code that still uses `parse_nested()` without exceptions can include [`<tao/pegtl/parse_nested.hpp>`](../include/tao/pegtl/parse_nested.hpp) directly; in that mode it performs the nested parse without catching and wrapping exceptions.

### Parse Modes

The `apply_mode` enumerators were renamed:

| PEGTL 3.x | PEGTL 4.x |
| --- | --- |
| `apply_mode::action` | `apply_mode::enabled` |
| `apply_mode::nothing` | `apply_mode::disabled` |

The `rewind_mode` values `active` and `dontcare` were merged into `rewind_mode::optional`.
The `parse()` default is now `rewind_mode::optional`; PEGTL 3.x used `rewind_mode::required`.
If client code depends on the input being rewound after `parse()` returns `false`, pass `rewind_mode::required` explicitly.

### Headers and Rules

The `contrib` include directory was removed.
Most former `contrib` facilities were moved to one of [actions](Action-Reference.md), [controls](Control-Reference.md), [debug](Debug-Facilities.md), [examples](Example-Reference.md), [extra or deprecated headers](Extra-Reference.md), [binary](Rule-Reference.md#binary) or [Unicode](Rule-Reference.md#unicode) rules, or [stream parsing](Stream-Parsing.md).
Update includes accordingly; for example:

| PEGTL 3.x include | PEGTL 4.x include |
| --- | --- |
| `<tao/pegtl/contrib/parse_tree.hpp>` | [`<tao/pegtl/extra/parse_tree.hpp>`](../include/tao/pegtl/extra/parse_tree.hpp) |
| `<tao/pegtl/contrib/nested_exceptions.hpp>` | [`<tao/pegtl/extra/nested_exceptions.hpp>`](../include/tao/pegtl/extra/nested_exceptions.hpp) |
| `<tao/pegtl/contrib/raw_string.hpp>` | [`<tao/pegtl/extra/raw_string.hpp>`](../include/tao/pegtl/extra/raw_string.hpp) |
| `<tao/pegtl/contrib/analyze.hpp>` | [`<tao/pegtl/debug/analyze.hpp>`](../include/tao/pegtl/debug/analyze.hpp) |
| `<tao/pegtl/contrib/trace.hpp>` | [`<tao/pegtl/debug/trace.hpp>`](../include/tao/pegtl/debug/trace.hpp) |
| `<tao/pegtl/contrib/json.hpp>` | [`<tao/pegtl/example/json.hpp>`](../include/tao/pegtl/example/json.hpp) |
| `<tao/pegtl/contrib/uint32.hpp>` | [`<tao/pegtl/binary/uint32.hpp>`](../include/tao/pegtl/binary/uint32.hpp) |
| `<tao/pegtl/contrib/integer.hpp>` | [`<tao/pegtl/deprecated/integer.hpp>`](../include/tao/pegtl/deprecated/integer.hpp) or [`<tao/pegtl/extra/charconv.hpp>`](../include/tao/pegtl/extra/charconv.hpp) |
| `<tao/pegtl/utf8.hpp>` | [`<tao/pegtl/unicode/utf8.hpp>`](../include/tao/pegtl/unicode/utf8.hpp) |
| `<tao/pegtl/change_state.hpp>` | [`<tao/pegtl/action/change_state.hpp>`](../include/tao/pegtl/action/change_state.hpp) |
| `<tao/pegtl/change_action.hpp>` | [`<tao/pegtl/action/change_action.hpp>`](../include/tao/pegtl/action/change_action.hpp) |
| `<tao/pegtl/change_action_and_state.hpp>` | [`<tao/pegtl/action/change_action_and_state.hpp>`](../include/tao/pegtl/action/change_action_and_state.hpp) |
| `<tao/pegtl/must_if.hpp>` | [`<tao/pegtl/control/must_if.hpp>`](../include/tao/pegtl/control/must_if.hpp) |
| `<tao/pegtl/discard_input.hpp>` | [`<tao/pegtl/stream/discard_input.hpp>`](../include/tao/pegtl/stream/discard_input.hpp) |
| `<tao/pegtl/discard_input_on_success.hpp>` | [`<tao/pegtl/stream/discard_input_on.hpp>`](../include/tao/pegtl/stream/discard_input_on.hpp) |
| `<tao/pegtl/discard_input_on_failure.hpp>` | [`<tao/pegtl/stream/discard_input_on.hpp>`](../include/tao/pegtl/stream/discard_input_on.hpp) |

The old `contrib/peg.hpp` and `contrib/predicates.hpp` headers were removed without direct replacements.
Also note that [`<tao/pegtl.hpp>`](../include/tao/pegtl.hpp) includes fewer auxiliary headers than before; include action, control, debug, extra, binary, unicode and stream headers explicitly when client code uses them.

PEGTL 4.0 moved the string-literal macros to `<tao/pegtl/pegtl_string.hpp>`, which was also included by `<tao/pegtl.hpp>`.
These macros and that header were removed in PEGTL 5.0; see [String Rules](#string-rules).

The depth and consumption action helpers were renamed: use `check_depth` instead of the old `limit_depth`, `check_consume` instead of `check_bytes`, and `limit_consume` instead of `limit_bytes`.
Their headers are in [`action/`](../include/tao/pegtl/action).

The `try_catch` and `try_catch_type` rules were renamed to [`try_catch_return_false`](Rule-Reference.md#try_catch_return_false-r-) and [`try_catch_type_return_false`](Rule-Reference.md#try_catch_type_return_false-e-r-), respectively.
Use [`try_catch_raise_nested`](Rule-Reference.md#try_catch_raise_nested-r-) and its variants when a caught exception should be rethrown as a [nested parse error](Errors-and-Exceptions.md#nested-exceptions).

Use the new core rule [`separated`](Rule-Reference.md#separated-s-r-) instead of the previous contrib rule `separated_seq`.
Use [`consume< N >`](Rule-Reference.md#consume-num-) for the old `bytes< N >` use case, and the type-specific `many` rules when matching a fixed number of objects of a particular input data type.

The existing terminal rules that take a value pack now require at least one value for `one`, `not_one`, `ione`, `ranges` and their PEGTL 3.x type-specific variants.
For empty positive matches such as `one<>`, `ione<>` or `ranges<>` that intentionally failed, use `failure` instead.
For empty negative matches such as `not_one<>` that intentionally matched any input object, use the corresponding `any` rule instead.

The old stream-oriented `discard` and `require` rules are now in the [stream parsing](Stream-Parsing.md#rules) headers.


## Version 3.0.0

* The build system needs to be configured for C++17 or later.
* The macro `TAO_PEGTL_NAMESPACE` now defines the fully qualified namespace and was changed from `pegtl` to `tao::pegtl`. When setting `TAO_PEGTL_NAMESPACE`, adapt as needed; in case of doubt, add `tao::` as prefix.
* The 0-based `byte_in_line` was replaced with the 1-based `column`.
* The control class template's `apply()` and `apply0()` must only be visible (e.g. via SFINAE) when an action class template function with the appropriate signature exists. See `tao::pegtl::normal` as example.
* If you have a use-case for [state](Rule-Reference.md#state-s-r-)'s `S::success()` to have an extended signature to get access to the current `apply_mode`, `rewind_mode`, *action*- and *control* class (template), please let us know and we'll re-add this feature.
* The compatibility macros starting with `TAOCPP_PEGTL_` were removed, the corresponding `TAO_PEGTL_`-prefixed macros must be used.
* The compatibility uppercase enumerators were removed, the corresponding lowercase enumerators must be used.
* The compatibility `peek_byte()` member functions were removed, the `peek_uint8()` member functions must be used.
* The compatibility header `changes.hpp` was removed, use the action-based `change_*.hpp` headers.
* The parse tree nodes provide `node->type` (a `std::string_view`) instead of `node->name()`.
* The parse tree nodes provide `node->string_view()` and `node->string()` instead of `node->content()`.


## Version 2.8.0

* All enumerators were changed from uppercase to lowercase.
* The enumerator `tracking_mode::IMMEDIATE` was renamed to `tracking_mode::eager`.
* The input-classes' `peek_byte()` member function was renamed to `peek_uint8()`.

Compatibility enumerators and functions are provided, they will be removed in version 3.0.0.


## Version 2.4.0

* The common prefix for all macros was changed from `TAOCPP_PEGTL_` to `TAO_PEGTL_`.

Compatibility macros are provided, they will be removed in version 3.0.0.


## Version 2.0.0

Some of the [changes](Changelog.md#version-200) for version 2.0.0 require modifications to any source using the PEGTL.
The good news is that the semantics of all parsing rules and grammars is the same as for versions 1.x.
Existing grammars will continue to work as before once the following list of mostly naming and namespace related changes is taken into account.

* Everything in the PEGTL is now in namespace `tao::pegtl`.
* The file name extensions were changed from `.hh` to `.hpp`.
* The main include file is now `<tao/pegtl.hpp>`, all other include files are `<tao/pegtl/*.hpp>`.
* The distribution of tasks between [the parse functions and input classes](Inputs-and-Parsing.md) was changed.
* The string macros have been renamed from `pegtl_(i)string_t` to `TAOCPP_PEGTL_(I)STRING`.
* The `begin()` member function of the input-classes is now called `current()`.
* The first argument to actions' `apply()` is now of type `tao::pegtl::internal::action_input< ... >`.

For flexibility and future compatibility it is recommended to "template over" the first argument to `apply()` as shown in [Actions and States](Actions-and-States.md#apply).

The `pegtl` part of the namespace can be controlled with the `TAOCPP_PEGTL_NAMESPACE` macro which is set in `include/tao/pegtl/config.hpp` if not defined previously.

Most of the other changes for version 2.0.0 extend or optimize the PEGTL without breaking compatibility.
Applications integrated more tightly with the PEGTL might require additional changes.

Please contact the authors at `taocpp(at)icemx.net` for any further questions when updating the PEGTL.


## Version 1.0.0

There were [many important changes](Changelog.md#version-100) leading up to version 1.0.0.
Please contact the authors at `taocpp(at)icemx.net` for any further questions when updating the PEGTL.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
