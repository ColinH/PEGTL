# Action Reference

The reference documentation for all included actions.
The actions related to [stream parsing](Stream-Parsing.md) are [documented here](Stream-Parsing.md#actions).
The actions that are part of the [debug facilities](Debug-Facilities.md) are [documented here](Debug-Facilities.md#trace-action).
The builder-style actions are documented on the [Builder Actions](Builder.md) page.


## Contents

* [Preamble](#preamble)
* [Actions](#actions)
* [Index](#index)


## Preamble

By [default](Introduction.md#namespaces) all actions reside in namespace `tao::pegtl`.

Only `include/tao/pegtl/nothing.hpp`, which defines `nothing` and `maybe_nothing`, is automatically included with `<tao/pegtl.hpp>`.
For all other actions the appropriate headers from [`include/tao/pegtl/action/`](../include/tao/pegtl/action) need to be included manually.

The action classes shown below are selectively attached in the usual way.
For example we can invoke [`change_action`](#change_action-a-) to switch the parsing run from `my_action` to `other_action` while parsing `some_rule` as follows.

```c++
template< typename Rule >
struct other_action;

template< typename Rule >
struct my_action
   : tao::pegtl::nothing< Rule >
{};

template<>
struct my_action< some_rule >
   : tao::pegtl::change_action< other_action >
{};
```

> [!IMPORTANT]
> Please remember that the current `apply_mode` enables or disables only the `apply()` and `apply0()` action functions.
> When an action has a `match()` function it will be called regardless of `apply_mode`.

Some actions have a `success()` function that is called when the rule the action was attached to succeeds.
In some cases there is a default implementation that can be replaced with a user-defined function, in other cases there is no default and a user-defined function has to be supplied.

In either case, the procedure to supply a user-defined `success()` is the same.

```c++
template< typename Rule >
struct my_action
   : tao::pegtl::nothing< Rule >
{ /* base definition */ }

struct action_that_calls_success;

template<>
struct my_action< some_rule >
   : action_that_calls_success
{
   template< typename ParseInput,
             typename... States >
   static void success( const ParseInput&, States&&... )
   {
      ...
   }
};
```

Alternatively an intermediate class can be created that inherits from `action_that_calls_success` and implements `success()`, but is not itself attached to a rule.
Then specializations of `my_action` would inherit from that intermediate class.
This approach requires more code but is more modular.

Note that the correct `success()` function will be called by `action_that_calls_success::match()` even though the function is not `virtual` because `action_that_calls_success::match()` does not call `action_that_calls_success::success()`.
Instead, assuming the currently matched rule is `Rule` and the current action class template is `Action`, it calls `Action< Rule >::success()`.

This pattern is used throughout the PEGTL and is the reason for all action and control functions being passed the current action and control classes as template parameters, to make functions in derived classes visible and callable.


## Actions

By [default](Introduction.md#namespaces) all action classes and class templates reside in namespace `tao::pegtl`.

###### [`add_guard< G >`](../include/tao/pegtl/action/add_guard.hpp)

* Creates an object of type `G` before parsing the rule `R` it is attached to.
* If `G` can be default-constructed it will be, or
* it will be constructed with the input and all previous states as arguments.
* If actions are enabled and `G` has a `success()` method it will be called with the input and all states if `R` succeeds.
* Included via `include/tao/pegtl/action/add_guard.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

The difference between `add_guard` and `add_state` is that `add_guard` does not add the newly constructed object to the states while parsing the rule it is attached to.

```c++
template< typename AddGuard >
struct add_guard
   : maybe_nothing
{
   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... >
             class Action,
             template< typename... >
             class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] static bool match( ParseInput& in, States&&... st );

   template< typename ParseInput,
             typename... States >
   static void success( const ParseInput& in, AddGuard& g, States&&... st );
};
```

###### [`add_state< S >`](../include/tao/pegtl/action/add_state.hpp)

* Creates an object of type `S` before parsing the rule `R` it is attached to.
* Adds this new object of type `S` as first state object while parsing `R`.
* If `S` can be default-constructed it will be, or
* it will be constructed with the input and all previous states as arguments.
* If actions are enabled and `S` has a `success()` method it will be called with the input and all states if `R` succeeds.
* Included via `include/tao/pegtl/action/add_state.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

The difference between `add_guard` and `add_state` is that `add_state` adds the newly constructed object as first state while parsing the rule it is attached to.

```c++
template< typename AddState >
struct add_state
   : maybe_nothing
{ ... };
```

###### [`change_action< A >`](../include/tao/pegtl/action/change_action.hpp)

* Parses the rule it is attached to substituting `A` as current action.
* Does **not** change the current `apply_mode`.
* Non-intrusive action equivalent of the [`action`](Rule-Reference.md#action-a-r-) rule.
* Included via `include/tao/pegtl/action/change_action.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< template< typename... > class NewAction >
struct change_action
   : maybe_nothing
{ ... };
```

###### [`change_action_and_state< A, S >`](../include/tao/pegtl/action/change_action_and_state.hpp)

* Parses the rule it is attached to substituting `A` as current action.
* Does **not** change the current `apply_mode`.
* Creates an object of type `S` before parsing the rule `R` it is attached to.
* Uses the new object of type `S` as **only** state object while parsing `R`.
* If `S` can be default-constructed it will be, or
* it will be constructed with the input and all previous states as arguments.
* If actions are enabled and `S` has a `success()` method it will be called with the input and all states if `R` succeeds.
* Combines [`change_action`](#change_action-a-) and [`change_state`](#change_state-s-) into a single action.
* Included via `include/tao/pegtl/action/change_action_and_state.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< template< typename... > class NewAction, typename NewState >
struct change_action_and_state
   : maybe_nothing
{ ... };
```

###### [`change_action_and_states< A, S... >`](../include/tao/pegtl/action/change_action_and_states.hpp)

* Combines [`change_action`](#change_action-a-) and [`change_states`](#change_states-s-) into a single action.
* Parses the rule it is attached to substituting `A` as current action.
* Does **not** change the current `apply_mode`.
* Creates objects of types `S...` before parsing the rule `R` it is attached to.
* Uses the new objects of types `S...` as state objects while parsing `R`.
* The new states are all default-constructed.
* If actions are enabled it calls `A< R >::success()` with the new and old states as arguments in that order.
* There is no default implementation for `success()`, it **must** be user defined.
* Included via `include/tao/pegtl/action/change_action_and_states.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< template< typename... > class NewAction, typename... NewStates >
struct change_action_and_states
   : maybe_nothing
{ ... };
```

###### [`change_control< C >`](../include/tao/pegtl/action/change_control.hpp)

* Parses the rule it is attached to substituting `C` as current control.
* Non-intrusive action equivalent of the [`control`](Rule-Reference.md#control-c-r-) rule.
* Included via `include/tao/pegtl/action/change_control.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< template< typename... > class NewControl >
struct change_control
   : maybe_nothing
{ ... };
```

###### [`change_rule< R >`](../include/tao/pegtl/action/change_rule.hpp)

* Parses this action's template parameter instead of the rule this action is attached to.
* Included via `include/tao/pegtl/action/change_rule.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< typename NewRule >
struct change_rule
   : maybe_nothing
{ ... };
```

> [!CAUTION]
> The change made by this action is invisible to the [grammar analysis](Debug-Facilities.md#grammar-analysis).

###### [`change_state< S >`](../include/tao/pegtl/action/change_state.hpp)

* Creates an object of type `S` before parsing the rule `R` it is attached to.
* Uses the new object of type `S` as **only** state object while parsing `R`.
* If `S` can be default-constructed it will be, or
* it will be constructed with the input and all previous states as arguments.
* If actions are enabled and `S` has a `success()` method it will be called with the input and all states if `R` succeeds.
* Included via `include/tao/pegtl/action/change_state.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< typename NewState >
struct change_state
   : maybe_nothing
{ ... };
```

###### [`change_states< S... >`](../include/tao/pegtl/action/change_states.hpp)

* Creates objects of types `S...` before parsing the rule `R` it is attached to.
* Uses the new objects of types `S...` as state objects while parsing `R`.
* The new states are all default-constructed.
* If actions are enabled it calls `A< R >::success()` with the new and old states as arguments in that order.
* There is no default implementation for `success()`, it **must** be user defined.
* Included via `include/tao/pegtl/action/change_states.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
template< typename... NewStates >
struct change_states
   : maybe_nothing
{ ... };
```

###### [`check_consume< N >`](../include/tao/pegtl/action/check_consume.hpp)

* Checks how many input objects the rule it is attached to consumed.
* Throws an exception when it consumed *more than* `N` input objects.
* Included via `include/tao/pegtl/action/check_consume.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).
* Similar to [`limit_consume`](#limit_consume-n-), but `check_consume` checks the consumption *after* matching.


```c++
 template< std::size_t Maximum >
 struct check_consume
    : maybe_nothing
 { ... };
```

###### [`check_depth< N >`](../include/tao/pegtl/action/check_depth.hpp)

* Limits the rule nesting depth for the rule(s) it is attached to.
* Throws an exception when the nesting depth exceeds the limit.
* Does not count rules when control is disabled.
* Requires an input with `make_depth_guard()` function like [`input_with_depth`](Input-Reference.md#input-with-depth).
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
 template< std::size_t Maximum >
 struct check_depth
    : maybe_nothing
 { ... };
```

###### [`control_action`](../include/tao/pegtl/action/control_action.hpp)

* Adds the `start()`, `success()` and `failure()` control functions to the action.
* Implements do-nothing default versions of these functions.
* Optionally adds the `unwind()` control function to the action.
* Included via `include/tao/pegtl/action/control_action.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
struct control_action
   : maybe_nothing
{ ... };
```

When using this action, all specializations `my_control< Rule >` that use `control_action` as base class have to implement the following member functions.

```c++
   template< typename ParseInput, typename... States >
   static void start( const ParseInput&, States&&... );

   template< typename ParseInput, typename... States >
   static void success( const ParseInput&, States&&... );

   template< typename ParseInput, typename... States >
   static void failure( const ParseInput&, States&&... );
```

The `unwind()` function is optional.
As in other cases where the PEGTL auto-detects the presence of `unwind()` its *absence* will lead to a micro-optimization (by omitting a guard object or a try-catch block).

```c++
   template< typename ParseInput, typename... States >
   static void unwind( const ParseInput&, States&&... );
```

###### [`disable_action`](../include/tao/pegtl/action/disable_action.hpp)

* Parses the rule it is attached to with actions disabled:
* Changes the current `apply_mode` to `apply_mode::disabled`.
* Non-intrusive action equivalent of the [`disable`](Rule-Reference.md#disable-r-) rule.
* Included via `include/tao/pegtl/action/disable_action.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
struct disable_action
   : maybe_nothing
{ ... };
```

###### [`enable_action`](../include/tao/pegtl/action/enable_action.hpp)

* Parses the rule it is attached to with actions enabled:
* Changes the current `apply_mode` to `apply_mode::enabled`.
* Non-intrusive action equivalent of the [`enable`](Rule-Reference.md#enable-r-) rule.
* Included via `include/tao/pegtl/action/enable_action.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).

```c++
struct enable_action
   : maybe_nothing
{ ... };
```

###### [`limit_consume< N >`](../include/tao/pegtl/action/limit_consume.hpp)

* Parses the rule it is attached to with the input limited to contain at most the next `N` objects.
* Throws an exception when the rule consumes all `N` input objects (unless there are no more input objects).
* Requires an input that implements `private_set_end()` like the [`view_input`](Input-Reference.md#view-input).
* Included via `include/tao/pegtl/action/limit_consume.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).
* Similar to [`check_consume`](#check_consume-n-), but `limit_consume` limits by modifying the input *before* matching.


```c++
 template< std::size_t Maximum >
 struct limit_consume
    : maybe_nothing
 { ... };
```

###### [`match_typed_state< T >`](../include/tao/pegtl/action/match_typed_state.hpp)

* Parses the rule it is attached to with only the state of type `T`.
* Included via `include/tao/pegtl/action/match_typed_state.hpp`.
* This action implements only a `match()` function and therefore:
* Publicly derives from [`maybe_nothing`](#maybe_nothing).
* Related to the [control](Control-and-Normal.md) class [`apply_typed_state`](Control-Reference.md#apply_typed_state).

```c++
template< typename T >
struct match_typed_state
   : maybe_nothing
{ ... };
```

###### [`maybe_nothing`](../include/tao/pegtl/nothing.hpp)

* A type alias for `tao::pegtl::nothing< void >`.
* Included via `include/tao/pegtl/nothing.hpp` or `include/tao/pegtl.hpp`.

This type (alias) serves as a tag type in two strongly related roles.
When `my_action< void >` has `tao::pegtl::nothing< void >` aka. `maybe_nothing` as public base class then the PEGTL will `static_assert` the presence of a callable `apply()` or `apply0()` function for all action class template specializations that do not also have `maybe_nothing` as public base class.

```c++
using maybe_nothing = nothing< void >;
```

###### [`nothing< R >`](../include/tao/pegtl/nothing.hpp)

* A "do nothing" action, defined as an empty class template.
* Included via `include/tao/pegtl/nothing.hpp` or `include/tao/pegtl.hpp`.
* The default action for [`parse()`](Inputs-and-Parsing.md#parse-function) and [`parse_nested()`](Inputs-and-Parsing.md#nested-parsing).

```c++
template< typename Rule >
struct nothing
{};
```

It is recommended, though not required, to use `nothing` as base class for the default case of custom action templates.
This enables the static assert explained with [`maybe_nothing`](#maybe_nothing).

```c++
template< typename Rule >
struct my_action
   : nothing< Rule >
{};

// ... (partial) specialisations of my_action ...
```

###### [`require_apply`](../include/tao/pegtl/action/require_apply.hpp)

* An empty tag class related to actions, not an action itself.
* Included via `include/tao/pegtl/action/require_apply.hpp`.

When an action (specialization for a rule) has `require_apply` as public base class the PEGTL statically asserts the presence of a callable `apply()` function in the action.
This check is performed independent of any presence or absence of `maybe_nothing`.

```c++
struct require_apply
{};
```

###### [`require_apply0`](../include/tao/pegtl/action/require_apply0.hpp)

* An empty tag class related to actions, not an action itself.
* Included via `include/tao/pegtl/action/require_apply0.hpp`.

When an action (specialization for a rule) has `require_apply0` as public base class the PEGTL statically asserts the presence of a callable `apply0()` function in the action.
This check is performed independent of any presence or absence of `maybe_nothing`.

```c++
struct require_apply0
{};
```


## Index

* [`add_guard`](#add_guard-g-) <sup>[(actions)](#actions)</sup>
* [`add_state`](#add_state-s-) <sup>[(actions)](#actions)</sup>
* [`change_action`](#change_action-a-) <sup>[(actions)](#actions)</sup>
* [`change_action_and_state`](#change_action_and_state-a-s-) <sup>[(actions)](#actions)</sup>
* [`change_action_and_states`](#change_action_and_states-a-s-) <sup>[(actions)](#actions)</sup>
* [`change_control`](#change_control-c-) <sup>[(actions)](#actions)</sup>
* [`change_rule`](#change_rule-r-) <sup>[(actions)](#actions)</sup>
* [`change_state`](#change_state-s-) <sup>[(actions)](#actions)</sup>
* [`change_states`](#change_states-s-) <sup>[(actions)](#actions)</sup>
* [`check_consume`](#check_consume-n-) <sup>[(actions)](#actions)</sup>
* [`check_depth`](#check_depth-n-) <sup>[(actions)](#actions)</sup>
* [`control_action`](#control_action) <sup>[(actions)](#actions)</sup>
* [`disable_action`](#disable_action) <sup>[(actions)](#actions)</sup>
* [`enable_action`](#enable_action) <sup>[(actions)](#actions)</sup>
* [`limit_consume`](#limit_consume-n-) <sup>[(actions)](#actions)</sup>
* [`match_typed_state`](#match_typed_state-t-) <sup>[(actions)](#actions)</sup>
* [`maybe_nothing`](#maybe_nothing) <sup>[(actions)](#actions)</sup>
* [`nothing`](#nothing-r-) <sup>[(actions)](#actions)</sup>
* [`require_apply`](#require_apply) <sup>[(actions)](#actions)</sup>
* [`require_apply0`](#require_apply0) <sup>[(actions)](#actions)</sup>


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
