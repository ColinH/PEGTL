# Builder Actions

Builder-style actions are predefined semantic actions for constructing C++ values while parsing.
They cover common tasks such as converting matched input, supplying constants, collecting repeated values, assembling records, selecting variant alternatives, and recursively creating objects.


## Contents

* [Introduction](#introduction)
  * [Choosing a Builder](#choosing-a-builder)
  * [From Callbacks to Builders](#from-callbacks-to-builders)
  * [A Composed Value](#a-composed-value)
  * [Mixing Builders and Manual Actions](#mixing-builders-and-manual-actions)
* [Reference](#reference)
  * [Headers](#headers)
  * [Targets and Delivery](#targets-and-delivery)
  * [Top-Level and Nested Forms](#top-level-and-nested-forms)
  * [Action Ownership](#action-ownership)
  * [Success, Failure, and Action Modes](#success-failure-and-action-modes)
  * [`build_to` and `build_for`](#build_to-and-build_for)
  * [`value_to` and `value_for`](#value_to-and-value_for)
  * [`const_to` and `const_for`](#const_to-and-const_for)
  * [`multi_to` and `multi_for`](#multi_to-and-multi_for)
  * [`optional_to` and `optional_for`](#optional_to-and-optional_for)
  * [`repeat_to` and `repeat_for`](#repeat_to-and-repeat_for)
  * [`variant_to` and `variant_for`](#variant_to-and-variant_for)
  * [`cases`, `cases_to`, and `cases_for`](#cases-cases_to-and-cases_for)
  * [`create_to` and `create_for`](#create_to-and-create_for)
  * [Customization Traits](#customization-traits)
  * [Nesting and Recursive Construction](#nesting-and-recursive-construction)
  * [Grammar Design for Builders](#grammar-design-for-builders)
  * [Configuration Constraints](#configuration-constraints)
  * [Common Pitfalls](#common-pitfalls)
  * [Extra Builders](#extra-builders)
* [JSON Walk-Through](#json-walk-through)
  * [The Data Model](#the-data-model)
  * [The Recursive Binding](#the-recursive-binding)
  * [The Variant Action](#the-variant-action)
  * [Arrays and Objects](#arrays-and-objects)
* [Custom Builders](#custom-builders)
  * [The Producer Protocol](#the-producer-protocol)
  * [A Simple Custom Builder](#a-simple-custom-builder)
  * [The Unescape Builders](#the-unescape-builders)
  * [The From-Chars Builders](#the-from-chars-builders)
  * [Multi-Rule Bindings](#multi-rule-bindings)


## Introduction

PEGTL [actions](Actions-and-States.md) are callbacks selected by grammar rule.
A manual action can inspect the matched input and update one or more parse states in any way the application requires.
Builder-style actions add a declarative layer for the common case where an action constructs a value and delivers it to one parse state.

Builders are ordinary PEGTL actions.
They use the same action class template, action specializations, parse states, controls, and `apply_mode` as manual actions.
An application can use builders for regular data construction and retain manual actions for validation, side effects, or semantics that do not fit a builder.

The facilities described on this page are only available when exception support is enabled.


### From Callbacks to Builders

Consider a rule whose matched input should be stored as a `std::string`.
A manual action could be written as follows.

```c++
struct result
{
   std::string name;
};

template< typename Rule >
struct action
   : tao::pegtl::nothing< Rule >
{};

template<>
struct action< name >
{
   template< typename ActionInput >
   static void apply( const ActionInput& in, result& out )
   {
      out.name = in.string();
   }
};
```

The equivalent builder action states the destination and lets the destination type select the conversion.

```c++
template<>
struct action< name >
   : tao::pegtl::value_to< &result::name >
{};
```

The member object pointer identifies both the target parse state (`result`) and the result type (`std::string`).
`value_to` converts the matched input to that type and assigns the result.

Targets are not limited to data members.
A builder can call a member function or a free function, which is particularly useful when several constructed values must be delivered together.


### A Composed Value

The following grammar describes a name and an integer separated by `=`.

```c++
struct name : tao::pegtl::plus< tao::pegtl::alpha > {};
struct number : tao::pegtl::plus< tao::pegtl::digit > {};
struct entry : tao::pegtl::seq< name, tao::pegtl::one< '=' >, number > {};
```

A single `multi_to` action captures the two subrules and calls `result::set()` only after the complete `entry` rule succeeds.

```c++
struct result
{
   std::string name;
   int number = 0;

   void set( std::string in_name, const int in_number )
   {
      name = std::move( in_name );
      number = in_number;
   }
};

template<>
struct action< entry >
   : tao::pegtl::multi_to< &result::set, name, number >
{};
```

Plain rule arguments such as `name` and `number` use the default `value_for` conversion.
More specific nested builders can be substituted without changing the surrounding `multi_to`.
For example, an escaped string can use `unescape_for< name >`, and a nested list can use `repeat_for< list_rule, element_rule >`.


### Mixing Builders and Manual Actions

Builder and manual action specializations can coexist in one action class template.

```c++
template<>
struct action< name >
   : tao::pegtl::value_to< &result::name >
{};

template<>
struct action< warning >
{
   template< typename ActionInput >
   static void apply( const ActionInput& in, result& out )
   {
      out.warnings.emplace_back( in.position(), in.string() );
   }
};
```

Whether a nested manual action runs depends on which builder owns the enclosing rule.
`value_to` and `const_to` are regular `apply()` and `apply0()` actions, respectively, so actions attached to their subrules continue to run.
The composing builders use `match()` and deliberately own the action processing below their boundary; this distinction is described in [Action Ownership](#action-ownership).

### Choosing a Builder

Choose a builder according to the shape of the C++ value to construct.

| Intent | Builder |
|--|--|
| Convert the complete matched input to one value | `value_to` / `value_for` |
| Produce a fixed value without inspecting the input | `const_to` / `const_for` |
| Assemble one value from several captured fields | `multi_to` / `multi_for` |
| Let one captured rule control whether a value is present | `optional_to` / `optional_for` |
| Collect zero or more elements | `repeat_to` / `repeat_for` |
| Select one `std::variant` alternative | `variant_to` / `variant_for` |
| Map grammar alternatives to fixed values | `cases`, `cases_to`, and `cases_for` |
| Parse a fresh object with its own action template | `create_to` / `create_for` |
| Define another construction policy | `build_to` / `build_for` |

Use the `_to` form when the completed value should be delivered to a parse state.
Use the `_for` form when it is one field, element, or alternative of another builder.
When a nested value only needs normal matched-input conversion, pass its grammar rule directly.


## Reference


### Headers

The standard builders and the three customization traits are provided by:

```c++
#include <tao/pegtl/action/builders.hpp>
```

The generic extension primitives can also be included separately:

```c++
#include <tao/pegtl/action/build_for.hpp>
#include <tao/pegtl/action/build_to.hpp>
```

Builders that integrate the extra `charconv` and `unescape` facilities are provided by:

```c++
#include <tao/pegtl/extra/builders.hpp>
```


### Targets and Delivery

The first template parameter `S` of every `_to` builder specifies where the completed value is delivered.
The target can be a member object pointer, a member function pointer, or a free function pointer.

```c++
struct result
{
   int value = 0;

   void set_value( const int in )
   {
      value = in;
   }
};

void set_value( result& out, const int in )
{
   out.value = in;
}

using member_object_builder = tao::pegtl::value_to< &result::value >;
using member_function_builder = tao::pegtl::value_to< &result::set_value >;
using free_function_builder = tao::pegtl::value_to< &set_value >;
```

For a free function, the first parameter is a mutable reference to the target state type.
The remaining parameters describe the values to construct.
A member function's object is the target parse state and all of its parameters describe constructed values.
Delivery functions return `void`; they can be either throwing or `noexcept`.
Their constructed arguments are delivered as temporary values, so value, const-reference, and rvalue-reference parameters are suitable; non-const lvalue-reference parameters are not.

For a member object pointer, the constructed result type is the member type.
For a delivery function with one value parameter, it is the decayed parameter type.
For a delivery function with zero or multiple value parameters, the constructed result is represented internally by a `std::tuple` of the decayed parameter types.
This lets a composing builder construct all arguments before making one final call.

The parse states passed to `parse()` must contain exactly one object of the target type.
Their order is irrelevant.
Having no target object, or multiple parse states of the same target type, is a compile-time error.


### Top-Level and Nested Forms

Names ending in `_to` are actions that deliver a completed result to `S`.
They are attached to a rule through the application's action class template.

```c++
template<>
struct action< number >
   : tao::pegtl::value_to< &result::value >
{};
```

Names ending in `_for` describe a value produced by a `Rule` for use inside another builder.
They are bindings rather than actions and are passed as arguments to `multi_to`, `optional_to`, `repeat_to`, `variant_to`, or their nested forms.

```c++
template<>
struct action< values >
   : tao::pegtl::repeat_to<
        &result::values,
        tao::pegtl::value_for< number > >
{};
```

A plain grammar rule is shorthand for `value_for< Rule >`, so the usual spelling is shorter.

```c++
template<>
struct action< values >
   : tao::pegtl::repeat_to< &result::values, number >
{};
```

The `_for` forms compose recursively.
For example, a repeated element can be a multi-field object, a variant alternative can be a repeated container, and a multi-field object can contain an optional or another container.
The C++ type expected by the surrounding builder determines the result type of the nested builder.

Builder names describe the shape of the constructed C++ value, not a required grammar combinator.
`multi_to` does not require `seq<>`, `variant_to` does not require `sor<>`, `optional_to` does not require `opt<>`, and `repeat_to` can collect matches produced by `star<>`, `plus<>`, `list<>`, `until<>`, or recursive rules.

`cases< Cases... >` is the one standard binding without a boundary `Rule`.
It can be triggered by any of the rules named by its cases.


### Action Ownership

> [!IMPORTANT]
> `value_to` and `const_to` do not disable actions on subrules of the rule to which they are attached.
> All other standard `_to` builders are based on `build_to` and disable the application's actions below their boundary.

`value_to` implements `apply()` and `const_to` implements `apply0()`.
The PEGTL first matches their rule with the normal action template, including actions on subrules, and then invokes the builder callback.
Attaching `value_to` to an outer rule and another action to one of its subrules therefore invokes both actions.

`build_to`-based builders implement `match()`.
They replace the application's actions below the attached rule with the actions declared by their bindings.
Rules selected by a plain argument use value conversion; explicit `_for` arguments use their selected producer; rules not selected by a binding have no action.

There are two deliberate variations:

* `create_to` and `create_for` parse their boundary with the explicitly supplied `ObjectAction` template.
* `unescape_to` and `unescape_for` parse their boundary with the supplied unescape action template.

This ownership makes a composed builder self-contained and avoids duplicate side effects from an application action and a nested producer processing the same rule.
Manual actions outside the builder boundary continue to run normally.
When manual behavior is required inside a composed value, it should be expressed by a custom producer or, for a separately constructed object, by the `ObjectAction` supplied to `create_to` or `create_for`.


### Success, Failure, and Action Modes

A `build_to`-based builder constructs its result in local temporary storage.
It delivers the result only after its complete boundary rule succeeds.
If the boundary fails, the temporary result is discarded and the target is unchanged.
This applies recursively: a nested value is committed to its surrounding temporary only after the nested boundary succeeds.

Direct `value_to` and `const_to` actions have the usual PEGTL action semantics.
They run after their own rule succeeds, but a later failure of an enclosing rule does not undo their effects.
The same caveat applies to manual actions.

If `apply_mode::disabled` is selected, builders perform no conversion, construction, or delivery.
The grammar is matched with the original action template and the disabled apply mode.
As elsewhere in the PEGTL, this disables `apply()` and `apply0()` callbacks but does not suppress unrelated actions that implement `match()`.

Input errors detected during conversion or insertion throw parse errors.
Examples include a number outside the target range and duplicate values for a `std::set` or duplicate keys for a `std::map`.
In contrast, missing captures, duplicate captures where only one is expected, an unselected variant, or multiple selected variant alternatives indicate a mismatch between the grammar and builder specification; these are programming errors guarded by compile-time checks or assertions.


### `build_to` and `build_for`

`build_to` and `build_for` are the generic primitives used to create additional builder families.

```c++
template< auto S, typename Producer >
struct build_to;

template< typename Rule, typename Producer >
struct build_for;
```

`build_to< S, Producer >` is an action.
It asks `Producer` to construct the result type required by `S`, matches the attached rule with the producer's action, and delivers the produced value after success.
It owns actions below its boundary as described above.

`build_for< Rule, Producer >` is a one-rule binding for use inside a composing builder.
It does not deliver to a parse state and is not itself attached as an action.
When `Rule` matches inside the surrounding builder, the producer constructs the value required for that argument position.

The producer protocol is intentionally small and is documented in [Custom Builders](#custom-builders).
A producer can use an ordinary `apply()` or `apply0()` action, or a `match()` action when it needs control over how the rule is parsed.


### `value_to` and `value_for`

```c++
template< auto S > struct value_to;
template< typename Rule > using value_for = ...;
```

`value_to` converts the entire matched input to the result type required by `S` and delivers it.
`value_for` performs the same conversion for one nested argument.
As noted above, a plain rule in a composed builder is shorthand for `value_for< Rule >`.

Conversion is selected through `value_traits< T >`.
The supplied specializations support:

* `char`, for input containing exactly one character,
* `std::string`,
* `std::string_view`,
* `std::vector< char >`, treated as one atomic value rather than a repeated container,
* `float` and `double`,
* `signed char`, `signed short`, `signed int`, `signed long`, and `signed long long`,
* their unsigned counterparts, and
* `std::optional< T >` when `value_traits< T >` is available.

Floating-point and integer conversions must consume the complete matched input.
Conversion failures and range errors throw a parse error associated with the matched input.
The floating-point conversions use `std::stof()` and `std::stod()`; the standard integer conversions use the decimal forms of `std::stoll()` and `std::stoull()`.
The grammar should define the desired lexical syntax before the builder performs this conversion.
`bool` is intentionally not parsed as an integer; use `const_to` or `cases_to` for textual booleans.

`value_to` applied to a `std::optional< T >` always produces an optional that contains a value because its rule has already matched.
Use `optional_to` when the presence of a separate subrule should decide whether the delivered optional contains something.

A stored `std::string_view` refers to the parse input.
The input storage must outlive every view retained by the result.

Applications can add conversions by specializing `value_traits`.

```c++
struct token_length
{
   std::size_t value = 0;
};

template<>
struct tao::pegtl::value_traits< token_length >
{
   template< typename ActionInput >
   [[nodiscard]] static token_length convert( const ActionInput& in ) noexcept
   {
      return { in.size() };
   }
};
```

The specialization returns a complete `T` and can inspect any operation offered by the action input.


### `const_to` and `const_for`

```c++
template< auto S, auto... Values > struct const_to;
template< typename Rule, auto... Values > using const_for = ...;
```

`const_to` constructs the result required by `S` from `Values...` whenever its rule succeeds.
`const_for` does the same for a nested argument.
The values are compile-time values, but the result type comes from the target or surrounding builder.

```c++
template<>
struct action< done >
   : tao::pegtl::const_to< &result::finished, true >
{};
```

The parameter pack can be empty, contain one value, or contain several values as long as the result type can be constructed from it.
An empty pack can default-construct a member value or invoke a zero-argument delivery function.
Several values can, for example, construct a `std::pair` member or provide all parameters of a delivery function.

`const_to` is a direct `apply0()` action and therefore preserves actions on subrules.
`const_for`, when used inside a composing builder, is owned by that surrounding builder.


### `multi_to` and `multi_for`

```c++
template< auto S, typename... Args > using multi_to = ...;
template< typename Rule, typename... Args > using multi_for = ...;
```

`multi_to` captures the values described by `Args...`, constructs one result from them, and delivers it after the boundary succeeds.
`multi_for` constructs the same kind of result for one nested argument.

The argument order is the construction and delivery order.
It does not have to match the order in which the grammar encounters the rules.

```c++
template<>
struct action< reversed >
   : tao::pegtl::multi_to<
        &result::set,
        second_number,
        first_name >
{};
```

The result shape is selected through `multi_traits< T >`.
The supplied specializations treat `std::pair< T, U >` as two fields and `std::tuple< Ts... >` as one field per element.
The default trait treats any other `T` as a one-field result containing `T`.
Consequently, `multi_to` has no special minimum arity: the number of arguments must exactly match `multi_traits< T >::fields_t`.
One-element tuples are supported, as are other arities described by a trait specialization.

For a delivery function with several value parameters, the delivery machinery represents those parameters as a tuple, so `multi_to` obtains their field types automatically.
For a member object of pair, tuple, or custom record type, `multi_traits` describes how that object is assembled.

```c++
struct entry_value
{
   std::string name;
   int number;
};

template<>
struct tao::pegtl::multi_traits< entry_value >
{
   using fields_t = tao::pegtl::type_list< std::string, int >;
};
```

After all fields are captured, the builder normally constructs `T` as `T( fields... )`.
This supports matching constructors and, with C++20 or later, parenthesized aggregate initialization.
If the trait supplies a compatible static `make()` function, it is used instead.

```c++
template<>
struct tao::pegtl::multi_traits< entry_value >
{
   using fields_t = tao::pegtl::type_list< std::string, int >;

   [[nodiscard]] static entry_value make( std::string name, const int number )
   {
      return { std::move( name ), number + 1 };
   }
};
```

Use `make()` when construction needs normalization, validation, a named factory, or behavior not expressed by a constructor.

An optional field has special absence semantics.
If a field type in `fields_t` is `std::optional< T >` and its binding does not match, `multi_to` passes `std::nullopt` for that field.
All non-optional fields must be captured exactly once before the boundary succeeds.


### `optional_to` and `optional_for`

```c++
template< auto S, typename Arg > using optional_to = ...;
template< typename Rule, typename Arg > using optional_for = ...;
```

These builders construct `std::optional< T >`.
The boundary rule decides whether the builder itself succeeds; `Arg` independently decides whether the completed optional contains a value.
The boundary does not need to be an `opt<>` rule.

```c++
struct maybe_number : tao::pegtl::seq< tao::pegtl::opt< number >, tao::pegtl::eof > {};

template<>
struct action< maybe_number >
   : tao::pegtl::optional_to< &result::number, number >
{};
```

When `maybe_number` succeeds, the target is always updated.
It receives an engaged optional if `number` matched and `std::nullopt` otherwise.
A delivery function is likewise called with `std::nullopt`; absence does not suppress the final call.
If the boundary itself fails, there is no delivery.

`optional_for` is useful when an optional is an element of a container or an alternative nested value.

```c++
template<>
struct action< sequence >
   : tao::pegtl::repeat_to<
        &result::values,
        tao::pegtl::optional_for< element, number > >
{};
```

Optional builders nest without flattening.
An `optional_for` that produces `std::optional< T >` can be the argument of an outer `optional_to` producing `std::optional< std::optional< T > >`.
If the outer boundary succeeds while the inner value is absent, the outer optional is engaged and contains `std::nullopt`.

For an optional field of `multi_to`, the field type itself already supplies absence handling; a plain rule is sufficient.
Use an explicit `optional_for` when the optional is itself the value expected by the surrounding builder.


### `repeat_to` and `repeat_for`

```c++
template< auto S, typename Arg > using repeat_to = ...;
template< typename Rule, typename Arg > using repeat_for = ...;
```

`repeat_to` default-constructs a result container, constructs one element each time `Arg` matches, and delivers the completed container after the boundary succeeds.
Zero matches produce an empty container.
The completed container replaces a member target; it is not appended to a previously stored container.

```c++
struct numbers : tao::pegtl::list< number, tao::pegtl::one< ',' > > {};

template<>
struct action< numbers >
   : tao::pegtl::repeat_to< &result::numbers, number >
{};
```

`repeat_for` produces a complete container as one nested value.
It permits containers of containers and containers used as fields of larger objects.

Insertion is selected through `repeat_traits< T >`.
Supplied specializations support:

* `std::vector< T >`,
* `std::list< T >`,
* `std::set< T >`,
* `std::multiset< T >`,
* `std::map< K, V >`, and
* `std::multimap< K, V >`,

including their allocator and comparator template parameters.
For maps, the element type constructed by `Arg` is `std::pair< K, V >`.
It is commonly produced with `multi_for< entry_rule, key_rule, value_rule >`.

`std::set` rejects duplicate values and `std::map` rejects duplicate keys by throwing a parse error.
The corresponding multi-containers retain duplicates.

Applications can support another container-like type by specializing `repeat_traits` with an `element_t` alias and an `add()` function.

```c++
struct bag
{
   std::vector< int > values;
};

template<>
struct tao::pegtl::repeat_traits< bag >
{
   using element_t = int;

   template< typename ParseInput >
   static void add( const ParseInput&, bag& out, int&& value )
   {
      out.values.emplace_back( value );
   }
};
```

The input passed to `add()` is positioned after the captured element.
The function can reject input-dependent insertion by throwing a parse error.


### `variant_to` and `variant_for`

```c++
template< auto S, typename... Args > using variant_to = ...;
template< typename Rule, typename... Args > using variant_for = ...;
```

These builders construct a `std::variant< Ts... >`.
There must be one argument for each alternative, in variant index order.
The grammar can encounter the alternative rules in a different order.

```c++
using scalar = std::variant< int, std::string, bool >;

template<>
struct action< scalar_rule >
   : tao::pegtl::variant_to<
        &result::value,
        integer,
        string_content,
        tao::pegtl::cases<
           tao::pegtl::case_< true_token, true >,
           tao::pegtl::case_< false_token, false > > >
{};
```

Exactly one alternative binding must match before the boundary succeeds.
No selected alternative or multiple selected alternatives indicate an invalid grammar/builder configuration.

Selection is by argument index rather than by C++ type.
Variant alternative types therefore do not need to be unique; `std::variant< int, int >` is supported when separate rule bindings select the two indices.

Each argument can itself be any nested builder.
A variant can therefore contain records, containers, optionals, or values built recursively with `create_for`.
Use `cases` when several grammar rules should all construct the same variant alternative.


### `cases`, `cases_to`, and `cases_for`

```c++
template< typename Rule, auto Value > struct case_;
template< typename... Cases > struct cases;

template< auto S, typename... Cases > using cases_to = ...;
template< typename Rule, typename... Cases > using cases_for = ...;
```

A case maps a rule type to a compile-time value.
The expected result type is supplied by the target or surrounding builder and is constructed from the selected value.

`cases_to` is attached to an outer rule containing all alternatives.

```c++
struct boolean : tao::pegtl::sor< false_token, true_token > {};

template<>
struct action< boolean >
   : tao::pegtl::cases_to<
        &result::flag,
        tao::pegtl::case_< false_token, false >,
        tao::pegtl::case_< true_token, true > >
{};
```

`cases_for< Rule, Cases... >` is the nested form when the case rules are enclosed by a named boundary `Rule`.

`cases< Cases... >` is a boundaryless binding.
It occupies one argument position but can be triggered directly by any of its case rules.
This is useful when alternatives occur directly in a surrounding `sor<>`, repeat, or multi-field grammar and adding another wrapper rule would serve no parsing purpose.

```c++
template<>
struct action< scalar_rule >
   : tao::pegtl::variant_to<
        &result::value,
        tao::pegtl::cases<
           tao::pegtl::case_< false_token, false >,
           tao::pegtl::case_< true_token, true > >,
        number >
{};
```

At least one case is required.
All case rules participate in the rule-uniqueness checks of the surrounding builder.


### `create_to` and `create_for`

```c++
template< auto S, typename Object, template< typename... > class ObjectAction >
using create_to = ...;

template< typename Rule, typename Object, template< typename... > class ObjectAction >
using create_for = ...;
```

`create_to` default-constructs a fresh `Object`, parses its boundary rule with `ObjectAction`, and delivers the object after success.
`create_for` performs the same operation for a nested argument.

```c++
struct child
{
   int value = 0;
};

template< typename Rule >
struct child_action
   : tao::pegtl::nothing< Rule >
{};

template<>
struct child_action< number >
   : tao::pegtl::value_to< &child::value >
{};

template<>
struct action< child_rule >
   : tao::pegtl::create_to< &result::child, child, child_action >
{};
```

While the boundary is parsed, the new object is the parse state visible to `ObjectAction`.
The previous parse states are not forwarded to that action template.
Actions in `ObjectAction`, including an action attached to the boundary rule itself, are enabled normally.

If parsing fails, the new object is discarded.
The object must be default-constructible and storable as the result expected by the surrounding builder.
The standard create builders construct objects directly; allocating smart pointers or creating a polymorphic object hierarchy requires a custom producer.

`create_for` is the standard recursion mechanism for pointer-free recursive values.
Each recursive match creates one fresh object on the parser's call stack; the maximum number of parse-state types remains a compile-time property and does not limit input nesting depth.


### Customization Traits

Three class templates customize how the standard builders handle application types.

| Trait | Used by | Required interface |
|--|--|--|
| `value_traits< T >` | `value_to`, `value_for`, and plain-rule bindings | `static T convert( const ActionInput& )` |
| `multi_traits< T >` | `multi_to` and `multi_for` | `fields_t`, and optionally `static T make( Fields... )` |
| `repeat_traits< T >` | `repeat_to` and `repeat_for` | `element_t` and `static void add( const ParseInput&, T&, element_t&& )` |

`value_traits` controls conversion of one complete match.
The supplied specializations cover strings, character data, floating-point values, signed and unsigned integers, and recursively engaged optionals.

`multi_traits` describes the ordered field types used to construct one result.
The default treats `T` as one field, while `std::pair` and `std::tuple` expose their elements.
A specialization can expose fields for an application type and can provide `make()` when direct construction is not sufficient.

`repeat_traits` describes one element type and how each completed element is added to a default-constructed result.
The supplied specializations cover the standard sequence, set, and map containers listed in the [`repeat_to` reference](#repeat_to-and-repeat_for).

A specialization affects both the `_to` and `_for` forms and therefore composes at every nesting level.
Specialize the narrowest applicable trait: use `value_traits` for atomic conversion, `multi_traits` for a fixed set of fields, and `repeat_traits` for an open-ended sequence of elements.


### Nesting and Recursive Construction

The composing builders are orthogonal: any `_for` result can be used wherever its C++ type matches the value expected by the surrounding builder.
For example, a `std::map< std::string, std::vector< int > >` can be constructed as a repeated map element whose second field is itself a repeated value.

```c++
template<>
struct action< groups >
   : tao::pegtl::repeat_to<
        &result::groups,
        tao::pegtl::multi_for<
           group,
           group_name,
           tao::pegtl::repeat_for< group_values, number > > >
{};
```

The same mechanism supports arbitrary non-recursive combinations of tuples, pairs, custom multi-field types, optionals, standard variants, and supported containers.
`create_for` adds recursive construction without changing the surrounding composition rules.

Nesting is controlled explicitly by the boundary rule of each `_for` binding.
The nested boundary must contain the rules captured by that nested builder.
This keeps the C++ construction tree visible in the action specification and independent of incidental parser state.


### Grammar Design for Builders

Composing builders use C++ rule types as capture identities.
Two fields that use the same textual syntax should therefore have distinct rule types when they belong to the same builder.

```c++
struct number : tao::pegtl::plus< tao::pegtl::digit > {};

struct first_number : number {};
struct second_number : number {};

struct range
   : tao::pegtl::seq< first_number,
                      tao::pegtl::one< ':' >,
                      second_number >
{};

template<>
struct action< range >
   : tao::pegtl::multi_to< &result::set_range,
                           first_number,
                           second_number >
{};
```

The derived rules need not change the accepted language.
Their purpose is to give actions and bindings unambiguous attachment points.

Choose a composing builder's boundary rule to represent the complete conceptual value being built.
Every rule captured directly by that builder must occur below the boundary, while nested conceptual values should receive their own `_for` boundary.
The boundary itself cannot also be a captured rule.

The builder argument list follows C++ construction order rather than grammar order.
Keep it in the order of function parameters, tuple elements, variant alternatives, or trait fields even when the grammar matches those rules in another order.

Repetition is the intentional exception to single capture.
A `repeat_to` has one element binding, and that binding can fire any number of times below the repeat boundary.
Do not represent repeated elements as several identical arguments; represent them as repeated matches of one argument rule.

Use `cases` when several alternative rule types represent one field without a useful common boundary.
Use `cases_for` when a named rule already provides a natural boundary around those alternatives.


### Configuration Constraints

The following constraints distinguish builder configuration errors from input errors.

* Within one composing builder, all rules contributed by its argument bindings must have distinct C++ rule types. A `cases` binding contributes every case rule.
* A composing builder's boundary rule cannot also be one of its captured argument rules.
* The argument count must match the number of fields, container element bindings, or variant alternatives expected by the result builder.
* Every field of `multi_to`, the argument of `optional_to`, and an alternative of `variant_to` can be captured at most once. Repeated capture is meaningful only for `repeat_to`.
* All required multi-fields and exactly one variant alternative must be captured before their boundary succeeds.
* The same textual grammar can be represented by distinct derived rule types when separate fields require distinct capture identities.
* These uniqueness rules apply within one composed builder. The same rule type can be used independently by unrelated builders.

Input order does not determine argument order.
The binding list determines where each captured rule is stored, so `multi_to< S, B, A >` works with a grammar `seq< A, B >` when the target expects the value from `B` first.


### Common Pitfalls

* **Expecting nested manual actions to run.** Only direct `value_to` and `const_to` preserve the application's actions on subrules. Composing builders own their boundary and run only their declared bindings or replacement action template.
* **Expecting rollback from a direct action.** A successful `value_to`, `const_to`, or manual callback is not undone when a later enclosing rule fails. Use a composing builder boundary when delivery must wait for the complete value.
* **Passing the wrong parse states.** Every `_to` target type must occur exactly once among the parse states. State order does not matter, but duplicate states of that type are ambiguous.
* **Reusing one rule type for several fields.** Capture identity is based on the C++ rule type, not on match position. Introduce distinct derived rules for distinct fields.
* **Allowing captures to be absent or duplicated incorrectly.** Missing multi-fields, duplicate optional captures, or missing or multiple variant alternatives are grammar/builder configuration errors, not ordinary parse failures.
* **Confusing optional conversion with optional presence.** `value_to` targeting `std::optional< T >` always produces an engaged optional. `optional_to` uses a separate argument rule to decide between a value and `std::nullopt`.
* **Expecting a repeat to append to existing data.** `repeat_to` builds a fresh container and delivers it after success. A member target is replaced; use a delivery function or custom trait for different merge semantics.
* **Assuming all containers accept duplicates.** `std::set` and `std::map` reject duplicate values or keys with a parse error; their multi-container counterparts retain them.
* **Retaining a dangling `std::string_view`.** Values produced through `value_traits< std::string_view >` refer to the input storage, which must outlive the result.
* **Expecting `create_to` actions to see outer states.** The supplied `ObjectAction` receives the newly created object as its parse state; previous parse states are deliberately not forwarded.


### Extra Builders

[`include/tao/pegtl/extra/builders.hpp`](Extra-Reference.md#buildershpp) provides four builders that integrate other extra facilities.

```c++
template< auto S > struct from_chars_to;
template< typename Rule > using from_chars_for = ...;

template< auto S, template< typename... > class Unescape = unescape >
using unescape_to = ...;

template< typename Rule, template< typename... > class Unescape = unescape >
using unescape_for = ...;
```

`from_chars_to` and `from_chars_for` work with the integer and floating-point rules from [`charconv.hpp`](Extra-Reference.md#charconvhpp), including `from_fchars_throws` and `from_fchars_nothrow`.
The output must be one of the supported signed or unsigned integer types, `float`, or `double`.
A typed rule must use the same numeric type; a rule instantiated with `void` obtains the type from the surrounding builder and must belong to the appropriate integer or floating-point family.
The rule's local-failure versus throwing range-error policy is preserved.
Parsing and conversion happen in one optimized operation.

`unescape_to` and `unescape_for` construct a `std::string` while parsing the selected rule with an unescape action template.
The default is the generic [`unescape`](Extra-Reference.md#unescapehpp) action.
A compatible action template can be supplied when a grammar defines different escaped-character rules.
As with `create_to`, only the newly created string is passed as parse state to that action template.


## JSON Walk-Through

The complete example is [`src/example/json_builder.cpp`](../src/example/json_builder.cpp).
It combines the JSON example grammar with a recursive C++ value using one builder specialization for `json::value`.


### The Data Model

```c++
struct value;

using array = std::vector< value >;
using object = std::map< std::string, value >;

struct value
{
   std::variant< std::nullptr_t, bool, double, std::string, array, object > data;
};
```

`value` contains six alternatives in a fixed order.
Arrays and objects contain `value` recursively, but the recursion passes through `std::vector` and `std::map`, so the type is well formed without pointers in the public data model.


### The Recursive Binding

The action template defaults to `nothing`, and a short alias describes how one nested JSON value is created.

```c++
template< typename Rule >
struct action
   : pegtl::nothing< Rule >
{};

template< typename Rule >
using json_value = pegtl::create_for< Rule, value, action >;
```

Whenever `json_value< Rule >` is selected by a surrounding builder, it default-constructs a fresh `value`, parses `Rule` with the same `action` template, and returns the completed value.
This is the recursive step used by both arrays and object members.


### The Variant Action

The specialization for the JSON grammar's `value` rule mirrors the six C++ variant alternatives.

```c++
template<>
struct action< rules::value >
   : pegtl::variant_to<
        &value::data,
        pegtl::const_for< rules::null, nullptr >,
        pegtl::cases< pegtl::case_< rules::false_, false >,
                      pegtl::case_< rules::true_, true > >,
        rules::number,
        pegtl::unescape_for< rules::string_content >,
        pegtl::repeat_for< rules::array, json_value< rules::array_element > >,
        pegtl::repeat_for<
           rules::object,
           pegtl::multi_for< rules::member,
                             pegtl::unescape_for< rules::key_content >,
                             json_value< rules::member_value > > > >
{};
```

The argument order is the `std::variant` index order:

* JSON `null` is converted to `std::nullptr_t` by `const_for`.
* The two Boolean grammar rules are grouped by boundaryless `cases`, and both select the `bool` alternative.
* `rules::number` is a plain rule that is automatically expanded to `value_for< rules::number >`; the expected `double` selects `value_traits< double >` for conversion.
* String content uses `unescape_for` to produce the `std::string` alternative.
* Arrays and objects use nested `repeat_for` builders.

The JSON grammar can encounter these alternatives in its own order.
The builder arguments are ordered by the data model, not by the `sor<>` definition.


### Arrays and Objects

For the array alternative, `repeat_for` constructs `std::vector< value >`.
Each `array_element` selects `json_value`, which creates and parses one recursive `value`.
An empty JSON array naturally produces an empty vector.

For the object alternative, `repeat_for` constructs `std::map< std::string, value >`.
Its element type is `std::pair< std::string, value >`, so `multi_for` captures two fields from each `member`:

* `unescape_for< key_content >` constructs the key, and
* `json_value< member_value >` recursively constructs the mapped value.

The completed pair is inserted into the map.
A duplicate JSON member name is rejected by the standard `repeat_traits< std::map<...> >` specialization.

The top-level parse receives one `value` state.
When the top-level `rules::value` succeeds, `variant_to` assigns the selected completed alternative to `value::data`.
Whitespace and punctuation rules have no bindings and therefore no semantic actions inside this builder boundary.


## Custom Builders

The public builder primitives are intended to support extensions that follow the same conventions as the supplied families.
A custom facility normally consists of a producer and two short public names:

```c++
template< auto S > using something_to = tao::pegtl::build_to< S, producer >;
template< typename Rule > using something_for = tao::pegtl::build_for< Rule, producer >;
```

The `_to` form is an action and the `_for` form is a nested binding.
Keeping both forms backed by the same producer guarantees that direct and nested uses perform the same conversion.


### The Producer Protocol

A producer supplies an action for a requested output slot.

```c++
struct producer
{
   template< typename Output >
   using action = producer_action< Output >;
};
```

`Output` provides:

* `Output::value_t`, the exact C++ type required by the target or enclosing builder, and
* `out.store( value_t&& )`, which stores the completed value.

On every successful match, the producer action must store exactly one result.
The producer action receives `Output&` as its first state, followed by the application's remaining parse states.
It can implement `apply()`, `apply0()`, or `match()` according to the usual PEGTL action protocol.

An `apply()` producer converts matched input after the rule succeeds.
An `apply0()` producer creates a value without examining matched input.
A `match()` producer can install another action template or arrange states while matching, but it is also responsible for preserving normal PEGTL matching, rewind, control, and `apply_mode` behavior.

> [!WARNING]
> A `match()` action must not call `Control< Rule >::match()` again with the unchanged `Action` template and the same parameters; that re-enters the same action indefinitely.
> Use the global `TAO_PEGTL_NAMESPACE::match()` when the boundary action must be bypassed, or call the control with a deliberately different action template as the supplied `create` and `from_chars` producers do.

Because `build_to` owns the boundary, actions from the application's action template are disabled below it.
A producer that needs subrule actions must explicitly install the action template it wants, as `create_to` and `unescape_to` do.


### A Simple Custom Builder

The following producer converts the length of the matched input to the requested result type.

```c++
struct length_producer
{
   template< typename Output >
   struct action
      : tao::pegtl::require_apply
   {
      template< typename ActionInput, typename... States >
      static void apply( const ActionInput& in,
                         Output& out,
                         States&&... )
      {
         using value_t = typename Output::value_t;
         out.store( value_t( in.size() ) );
      }
   };
};

template< auto S >
using length_to = tao::pegtl::build_to< S, length_producer >;

template< typename Rule >
using length_for = tao::pegtl::build_for< Rule, length_producer >;
```

`length_to` can assign a member or call a delivery function.
`length_for` can be used in any composing builder whose corresponding field or element type can be constructed from an input size.


### The Unescape Builders

The unescape builders demonstrate that an extension does not always need a new producer.
The existing `create_to` and `create_for` builders already express the required operation: create a string and parse a rule with an action template that fills it.

Their implementation is essentially:

```c++
template< auto S,
          template< typename... > class Unescape = tao::pegtl::unescape >
using unescape_to = tao::pegtl::create_to< S, std::string, Unescape >;

template< typename Rule,
          template< typename... > class Unescape = tao::pegtl::unescape >
using unescape_for = tao::pegtl::create_for< Rule, std::string, Unescape >;
```

This is the preferred kind of extension: compose existing public builders when they already provide the required lifetime, action ownership, and delivery behavior.


### The From-Chars Builders

The integer and floating-point `charconv` rules can parse and convert a number to their first state in one operation.
Using an ordinary `apply()` producer would discard that advantage by parsing first and converting the matched string a second time.

The `from_chars` producer therefore uses a `match()` action.
Conceptually it:

1. obtains the required numeric type from `Output::value_t`,
2. creates one numeric object,
3. matches the selected `charconv` rule with that number as its first state,
4. verifies that a typed rule agrees with the requested numeric type, and
5. moves the converted number into `out.store()` after success.

The public forms then use the same producer in the standard way.

```c++
template< auto S >
struct from_chars_to
   : tao::pegtl::build_to< S, from_chars_producer >
{};

template< typename Rule >
using from_chars_for = tao::pegtl::build_for< Rule, from_chars_producer >;
```

This example shows why the producer protocol accepts complete PEGTL actions rather than only conversion functions.
An extension can preserve an optimized rule-specific flow while remaining composable with every standard builder.


### Multi-Rule Bindings

`build_for` describes the common case where one rule selects one producer.
An advanced extension can define a binding that is selected by several rule types.
Such a binding provides:

```c++
struct binding
{
   using rules_t = tao::pegtl::type_list< rule_a, rule_b >;

   template< typename MatchedRule >
   using producer_t = producer_selected_for< MatchedRule >;
};
```

`cases<...>` is the standard example.
It lists all case rules in `rules_t` and selects a constant producer according to the actual `MatchedRule`.

All rules exposed by a custom binding participate in the surrounding builder's uniqueness checks.
Multi-rule bindings should therefore be reserved for one conceptual value that can be produced by several grammar alternatives.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
