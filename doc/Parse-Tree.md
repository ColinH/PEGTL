# Parse Tree

The PEGTL provides facilities for building a [parse tree](https://en.wikipedia.org/wiki/Parse_tree), or a more compact [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree)-like tree, while a parsing run is performed.

The parse tree support is considered an extra facility rather than a part of the core parser.
It can

* generate a *complete* tree with one node for every successful rule match,
* or select a subset of grammar rules that should become tree nodes.
* It can keep the matched input as a `std::string_view`,
* transform or discard nodes while the tree is being built, and
* use either the included node class or a user-provided one.


## Contents

* [Headers](#headers)
* [Full Parse Tree](#full-parse-tree)
* [Partial Parse Tree](#partial-parse-tree)
* [Transforming Nodes](#transforming-nodes)
* [Transformer Reference](#transformer-reference)
* [DOT Output](#dot-output)
* [Basic Tree Node](#basic-tree-node)
* [Custom Node Class](#custom-node-class)
* [Actions, Controls, and States](#actions-controls-and-states)
* [Requirements](#requirements)


## Headers

The parse tree builder has a header in the [extra](Extra-Reference.md) directory.

```c++
#include <tao/pegtl/extra/parse_tree.hpp>
```

The optional DOT printer for graph visualization has its own header.

```c++
#include <tao/pegtl/extra/parse_tree_to_dot.hpp>
```


## Full Parse Tree

To obtain a full parse tree, call `tao::pegtl::parse_tree::parse()` with a grammar and an input.

```c++
tao::pegtl::text_view_input in( "42" );

auto root = tao::pegtl::parse_tree::parse< my_grammar >( in );
```

The result is a `std::unique_ptr` to the root node.
The default node type is `tao::pegtl::parse_tree::node_t< ParseInput >`, where `ParseInput` is the input type after removing references and cv-qualifiers.

The pointer is empty when the top-level rule returns local failure, i.e. when a normal parsing run would return `false`.
If parsing succeeds, the returned root node is an artificial root whose `is_root()` member returns `true`; the matched grammar nodes are stored in `root->children`.
If parsing throws an exception, the exception propagates out of `parse_tree::parse()`.

The default node stores matched input in `data`, a `std::string_view` pointing into the original input.
The input object and its backing storage **must** therefore stay alive while `data` is used.


## Partial Parse Tree

Most useful parse trees contain only a subset of all grammar rules.
A *selector* is a class template whose instantiation for a rule determines whether that rule produces a node.

For each `Rule`, `my_selector< Rule >::value` must be a `bool` that controls whether a node is created.
Here is an example for a selector that only creates a tree node for the `number` rule.

```c++
template< typename Rule >
struct my_selector
   : std::false_type
{};

template<>
struct my_selector< number >
   : std::true_type
{};
```

The included `tao::pegtl::parse_tree::selector` provides additional functionality by combining multiple selectors, each of which selects a different collection of rules.
Each collection is usually created with `some_selector::on< Rules... >` where `some_selector` is usually a *transformer*, a selector that performs additional actions on the created tree nodes.

```c++
template< typename Rule >
using my_selector = tao::pegtl::parse_tree::selector<
   Rule,
   tao::pegtl::parse_tree::store_content::on<
      number,
      identifier >,
   tao::pegtl::parse_tree::remove_content::on<
      plus,
      minus > >;

auto root = tao::pegtl::parse_tree::parse< my_grammar, my_selector >( in );
```

In this example `number` and `identifier` nodes keep the matched input in `data`, while `plus` and `minus` nodes are kept as structural nodes without that content.
Rules that are not mentioned do not produce nodes of their own; selected descendants are still attached to the nearest selected ancestor.

If more than one collection matches the same rule, the selector triggers a compile-time error.


## Transforming Nodes

When a selected rule succeeds, the parse tree builder calls the node's `success()` function, then calls the selector's optional `transform()` function for that rule.
The transform receives the node as a `std::unique_ptr` and can modify the node, reorder or move its children, remove its content, replace it with one of its children, or reset the pointer to discard the node and all of its children.

For example the following `fold_one` selector (that is also a transformer since it defines a `transform()` function) will replace the node with an only child, or remove the content if there is not exactly one child.

```c++
struct fold_one
   : // implementation defined
{
   template< typename Node, typename... States >
   static void transform( std::unique_ptr< Node >& n, States&&... st )
   {
      if( n->children.size() == 1 ) {
         n = std::move( n->children.front() );
      }
      else {
         n->remove_content( st... );
      }
   }
};
```

A transform can also request the current parse input as first argument simply by including it in the argument list.

```c++
template< typename ParseInput, typename Node, typename... States >
static void transform( const ParseInput& in, std::unique_ptr< Node >& n, States&&... st );
```

The example program [`src/example/parse_tree.cpp`](../src/example/parse_tree.cpp) contains a larger transform that rearranges a simple expression parse tree into an AST-like binary operator tree.


## Transformer Reference

Several predefined selectors are available for common tree-building choices.
They are used as collections for `parse_tree::selector`.

###### `tao::pegtl::parse_tree::store_content`

Stores the node and leaves `data` set to the matched input.

```c++
tao::pegtl::parse_tree::store_content::on< Rules... >
```

###### `tao::pegtl::parse_tree::remove_content`

Stores the node, then calls `n->remove_content()`.
The node keeps its type, position, and children, but its `data` view becomes empty.

```c++
tao::pegtl::parse_tree::remove_content::on< Rules... >
```

###### `tao::pegtl::parse_tree::fold_one`

Stores the node.
If it has exactly one child, replaces the node with that child.
Otherwise it keeps the node and removes its content.

```c++
tao::pegtl::parse_tree::fold_one::on< Rules... >
```

###### `tao::pegtl::parse_tree::discard_empty`

Stores the node.
If it has no children, discards the node.
Otherwise it keeps the node and removes its content.

```c++
tao::pegtl::parse_tree::discard_empty::on< Rules... >
```

###### `tao::pegtl::parse_tree::apply< T >`

Adapts a user-defined transformer `T` so it can be used with `selector`.
The transformer type must provide the appropriate `transform()` function.

```c++
struct my_transform
   : tao::pegtl::parse_tree::apply< my_transform >
{
   template< typename Node, typename... States >
   static void transform( std::unique_ptr< Node >& n, States&&... st );
};

template< typename Rule >
using my_selector = tao::pegtl::parse_tree::selector<
   Rule,
   my_transform::on< expression, product > >;
```


## DOT Output

The `parse_tree_to_dot.hpp` helper prints a tree in [DOT](https://graphviz.org/doc/info/lang.html) format.

```c++
if( const auto root = tao::pegtl::parse_tree::parse< my_grammar, my_selector >( in ) ) {
   tao::pegtl::parse_tree::print_dot( std::cout, *root );
}
```

The resulting DOT can be rendered with [Graphviz](https://graphviz.org/).

```sh
./my_parser '{"foo":[42,null]}' | dot -Tsvg -o parse_tree.svg
```

The program [`src/example/json_parse_tree.cpp`](../src/example/json_parse_tree.cpp) demonstrates this with the included [JSON grammar](Example-Reference.md#jsonhpp).


## Basic Tree Node

The included node implementation is `basic_node< T, Position >`.
The default node type used by `parse_tree::parse()` is `node_t< ParseInput >`, which derives from `basic_node< node_t< ParseInput >, typename ParseInput::error_position_t >`.

The public interface is equivalent to the following outline.

```c++
template< typename T, typename Position >
struct basic_node
{
   using node_t = T;
   using children_t = std::vector< std::unique_ptr< node_t > >;

   children_t children;
   std::string_view type;
   std::string_view data;

   Position begin;
   Position end;

   bool is_root() const noexcept;

   template< typename Rule >
   bool is_type() const noexcept;

   template< typename Rule >
   void set_type() noexcept;

   std::string string() const;

   template< typename... States >
   void remove_content( States&&... st ) noexcept;

   template< typename Rule, typename ParseInput, typename... States >
   void start( const ParseInput& in, States&&... st );

   template< typename Rule, typename ParseInput, typename... States >
   void success( const ParseInput& in, States&&... st ) noexcept;

   template< typename Rule, typename ParseInput, typename... States >
   void failure( const ParseInput& in, States&&... st ) noexcept;

   template< typename... States >
   void emplace_back( std::unique_ptr< node_t >&& child, States&&... st );
};

template< typename ParseInput >
struct node_t
   : basic_node< node_t< ParseInput >, typename ParseInput::error_position_t >
{};
```

For non-root nodes, `type` contains the demangled rule name, `begin` and `end` contain the input positions before and after the match, and `data` references the matched input.
For the root node, `type` and `data` are empty and `is_root()` returns `true`.

Calling `string()` returns a copy of `data`.
Empty `data` can mean either that the rule matched an empty range, or that a transformer removed the node content.


## Custom Node Class

For more control over allocation or storage, pass a custom node class as the second template argument to `parse_tree::parse()`.

```c++
auto root = tao::pegtl::parse_tree::parse< my_grammar, my_node, my_selector >( in );
```

A custom node is owned through `std::unique_ptr`, is default-constructed by the parse tree builder, and must provide the callbacks used by the builder.

```c++
struct my_node
{
   using node_t = my_node;
   std::vector< std::unique_ptr< node_t > > children;

   my_node() = default;
   my_node( const my_node& ) = delete;
   my_node( my_node&& ) = delete;
   my_node& operator=( const my_node& ) = delete;
   my_node& operator=( my_node&& ) = delete;
   ~my_node() = default;

   template< typename Rule, typename ParseInput, typename... States >
   void start( const ParseInput& in, States&&... st );

   template< typename Rule, typename ParseInput, typename... States >
   void success( const ParseInput& in, States&&... st );

   template< typename Rule, typename ParseInput, typename... States >
   void failure( const ParseInput& in, States&&... st );

   template< typename... States >
   void emplace_back( std::unique_ptr< node_t >&& child, States&&... st );

   // Optional. Called when an exception unwinds through a selected rule.
   template< typename Rule, typename ParseInput, typename... States >
   void unwind( const ParseInput& in, States&&... st );
};
```

The `unwind()` callback is optional.
If present, it is called when stack unwinding passes through a selected rule whose node has already been started.
Custom nodes used with `remove_content`, `fold_one`, or `discard_empty` also need a compatible `remove_content()` member function.

Custom nodes often derive from `basic_node` and add the behavior they need.
For example, [`src/example/parse_tree.cpp`](../src/example/parse_tree.cpp) derives from `basic_node` to demonstrate custom allocation.


## Actions, Controls, and States

The `parse_tree::parse()` functions mirror the ordinary `parse()` customization points.
After the grammar and optional selector or node type, the remaining template parameters are the [action](Actions-and-States.md) and [control](Control-and-Normal.md) class templates.

```c++
   template< typename Rule,
             typename Node,
             template< typename... > class Selector = internal::store_all,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   [[nodiscard]] std::unique_ptr< Node > parse( ParseInput&& in, States&&... st );

   template< typename Rule,
             template< typename... > class Selector = internal::store_all,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             typename ParseInput,
             typename... States >
   [[nodiscard]] auto parse( ParseInput&& in, States&&... st );
```

The user states are forwarded to actions, controls, node callbacks, and transforms.
The parse tree builder adds a state object to the parsing run to keep track of the parse tree.

That state is hidden from actions and controls by the generated [control adapter](Control-Reference.md) that makes use of [`rotate_states_right`](Control-Reference.md#rotate_states_right) and [`remove_first_state`](Control-Reference.md#remove_first_state).
As usual, the additional state can not be hidden from the `match()` control (and action) functions.
The minimal example [`src/example/parse_tree_user_state.cpp`](../src/example/parse_tree_user_state.cpp) shows a user state being forwarded through parse tree parsing to an action.


## Requirements

The parse tree builder uses the `subs_t` rule metadata described in [Rules and Grammars](Rules-and-Grammars.md#type-aliases) and [Debug Facilities](Debug-Facilities.md#meta-data) to avoid creating internal stack nodes for branches that cannot contain selected descendants.
[Custom rules](Rules-and-Grammars.md#implementing-rules) must provide correct `rule_t` and `subs_t` type aliases for the parse tree builder to operate correctly.

Selected nodes store views into the input by default.

> [!CAUTION]
> When using [stream inputs](Stream-Parsing.md#inputs) or inputs whose backing storage can be discarded, make sure the selected node data remains valid for as long as the tree is inspected, or use a custom node that copies the data it needs.


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2018-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
