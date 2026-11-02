# Rules and Grammars

A (parsing) rule is a class that models a [(production) rule](https://en.wikipedia.org/wiki/Production_(computer_science)) of a [formal grammar](https://en.wikipedia.org/wiki/Formal_grammar), or a [parser combinator](https://en.wikipedia.org/wiki/Parser_combinator).
A grammar is a set of one or more related parsing rules, with one (or more) designated top-level rules as entry point(s).


## Contents

* [Introduction](#introduction)
* [Creating Rules](#creating-rules)
* [Recursive Rules](#recursive-rules)
* [S-Expression Grammar](#s-expression-grammar)
* [Implementing Rules](#implementing-rules)
  * [Match Function](#match-function)
  * [Simple Match](#simple-match)
  * [Simple Example](#simple-example)
  * [Complex Match](#complex-match)
  * [Rewind Guard](#rewind-guard)
  * [Complex Example](#complex-example)
  * [Type Aliases](#type-aliases)
  * [Exceptions](#exceptions)
  * [Complete Example](#complete-example)
* [Rule Optimizations](#rule-optimizations)
  * [Backtracking](#backtracking)
  * [Whitespace](#whitespace)
  * [Combinations](#combinations)
* [Rule Comparisons](#rule-comparisons)
  * [Simple Combinators](#simple-combinators)
  * [Iterating Combinators](#iterating-combinators)
  * [Repeating Combinators](#repeating-combinators)
  * [List Combinators](#list-combinators)
  * [ASCII String Rules](#ascii-string-rules)
  * [ASCII Matching Rules](#ascii-matching-rules)


## Introduction

A (parsing) rule is a class (or a class template) with a (suitable) static `match()` function (and some type aliases).
This match function attempts to match a portion of the input against some condition and either succeeds, possibly consuming a portion of the input, or fails.

A grammar is a set of one or more related parsing rules including an informally designated top-level rule that serves as entry point.
The top-level rule is *also* sometimes called a grammar.

Some grammars have more than one top-level rule; in theory any rule can be used as top-level rule -- the rule that drives a parsing run -- which can be useful to individually test the rules of a grammar.

The rules provided with the PEGTL are either classes with fixed parsing behavior or parameterizable class templates.
Rules that take other rules as parameters are also called (parsing) combinator rules, or *combinators*.
These rules correspond to non-terminals in [formal grammars](https://en.wikipedia.org/Formal_grammar).
Rules without template parameters, or only with template parameters that are not also rules, are also called *atomic* rules.
In [formal grammars](https://en.wikipedia.org/Formal_grammar) these are the terminals.

The rule [`tao::pegtl::eof`](Rule-Reference.md#eof) is declared as `struct eof`, a rule class without parameters.

The rule [`tao::pegtl::consume`](Rule-Reference.md#consume-num-) is declared as `template< std::size_t Num > struct consume`, a rule class template with a value parameter.

The rule [`tao::pegtl::seq`](Rule-Reference.md#seq-r-) is declared as `template< typename... Rule > struct seq`, a rule class template with other rule classes as type parameters, which makes it a combinator.

Creating custom rules and grammars is done by parameterizing and combining the rules provided with the PEGTL.

When a grammar goes beyond what is (easily) achievable with this approach a fully custom rule can be [implemented from scratch](#implementing-rules).


## Creating Rules

The usual way to create a new rule is to define a `struct` that inherits from the desired implementation.

```c++
struct sign : tao::pegtl::one< '+', '-' > {};
```

Defining new types and inheriting the desired behavior instead of using type aliases is preferred for two main reasons.

1. Some debug facilities will output the (shorter) type name rather than the (longer) expansion.
2. Using new types makes it possible to attach different [actions](Actions-and-States.md) to "different names of the same rule".

For example given the two type aliases

```c++
using my_rule1 = tao::pegtl::one< '+', '-' >;
using my_rule2 = tao::pegtl::one< '+', '-' >;
```

an action `my_action<>` specialized as `my_action< my_rule1 >` will be applied on successful match of `my_rule1`, `my_rule2` and `tao::pegtl::one< '+', '-' >` because they are all just different names for the same type.
If on the other hand the two rules are defined via

```c++
struct my_rule1 : tao::pegtl::one< '+', '-' > {};
struct my_rule2 : tao::pegtl::one< '+', '-' > {};
```

then `my_action< my_rule1 >` will *not* be applied on successful match of `my_rule2` or ``tao::pegtl::one< '+', '-' >` because now they are all distinct types -- even while sharing the very same `match()` function implementation.

A rule defined as `struct` with inheritance can of course again be a class template that needs to be instantiated to yield an actual parsing rule.

```c++
template< char N >
struct n_and_n_plus_one : tao::pegtl::one< N, N + 1 > {};
```

As further example consider the rules from the [main PEGTL readme](../README.md).

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

Here two rules are defined in the way introduced earlier.

1. A rule for the sign of a number, **one** character which is either a plus or a minus.
2. A rule for an integer, a **seq**uence of an **opt**ional sign followed by one or more digits.

This is all to how new rules are created from existing rules, using C++ template syntax as embedded language.

See the [Rule Reference](Rule-Reference.md) for a complete list of all rules and combinators included with the PEGTL.

Some included "unofficial" rules can be found in `include/tao/pegtl/extra` as documented on the [Extra Reference](Extra-Reference.md) page.


## Recursive Rules

Recursion, or cycles in the grammar, can be implemented with a forward-declaration of one or more rules.

```c++
struct number
   : tao::pegtl::plus< tao::pegtl::digit > {};

struct addition;  // Forward declaration to break the cyclic dependency.

struct bracket
   : tao::pegtl::if_must< tao::pegtl::one< '(' >, addition, tao::pegtl::one< ')' > > {};

struct atomic
   : tao::pegtl::sor< number, bracket > {};

struct addition
   : tao::pegtl::list< atomic, tao::pegtl::one< '+' > > {};
```

In many cases the same grammar can be implemented either recursively *or* with some iterating combinators like [`star<>`](Rule-Reference.md#star-r-), [`until<>`](Rule-Reference.md#until-r-) or [`list<>`](Rule-Reference.md#list-r-s-).
Usually the version with iterating combinators should be preferred due to using less stack space.


## S-Expression Grammar

To give another example of what a small real-world grammar might look like, here is the grammar for a toy-version of S-expressions.
It only supports proper lists, symbols, comments, and numbers, which are non-empty sequences of ASCII digits.

The rule named `file` is the intended top-level rule of the grammar, i.e. the rule that is supplied as template argument to [the `parse()` function](Inputs-and-Parsing.md#parse-function) in order to start a parsing run with this grammar.

```c++
struct hash_comment
   : tao::pegtl::until< tao::pegtl::eolf > {};

struct list;

struct list_comment
   : tao::pegtl::if_must< tao::pegtl::at< tao::pegtl::one< '(' > >, tao::pegtl::disable< list > > {};

struct read_include
   : tao::pegtl::seq< tao::pegtl::one< ' ' >, tao::pegtl::one< '"' >, tao::pegtl::plus< tao::pegtl::not_one< '"' > >, tao::pegtl::one< '"' > > {};

struct hash_include
   : tao::pegtl::if_must< tao::pegtl::string< "include" >, read_include > {};

struct hashed
   : tao::pegtl::if_must< tao::pegtl::one< '#' >, tao::pegtl::sor< hash_include, list_comment, hash_comment > > {};

struct number
   : tao::pegtl::plus< tao::pegtl::digit > {};

struct symbol
   : tao::pegtl::identifier {};

struct atom
   : tao::pegtl::sor< number, symbol > {};

struct anything;

struct list
   : tao::pegtl::if_must< tao::pegtl::one< '(' >, tao::pegtl::until< tao::pegtl::one< ')' >, anything > > {};

struct normal
   : tao::pegtl::sor< atom, list > {};

struct anything
   : tao::pegtl::sor< tao::pegtl::space, hashed, normal > {};

struct main
   : tao::pegtl::until< tao::pegtl::eof, tao::pegtl::must< anything > > {};
```

In order to let a parsing run do more than return whether an input conforms to the grammar it is necessary to attach actions to some of its rules, as explained in [Actions and States](Actions-and-States.md).


## Implementing Rules

When a grammar goes beyond what is (easily) achievable through combination of existing rules it can be necessary to implement a custom rule or combinator from scratch.

### Match Function

The match function must adhere to either the simple or the complex interface.

The simple interface can often be used for leaf rules, i.e. rules that do not call other rules.

The complex interface is required when calling other rules and/or some static (the apply mode, rewind mode, Action and Control) or dynamic (the State arguments) state of the current parsing run needs to be accessed within the match function.

All match functions return `bool`, making for **three** possible outcomes:

1. *Success*, when the rule returns `true`.
2. *Local failure*, when the rule returns `false`.
3. *Global failure*, when the rule throws an exception.

Global failures are thus named because, unlike local failures, they do not allow for backtracking -- exceptions (usually) abort a parsing run.

When a parsing rule returns `false` it **must not** have consumed any input.
The PEGTL **requires**, but neither verifies nor enforces, adherence to this rule.

When a parsing rule throws an exception the PEGTL makes no assumptions and provides no guarantees regarding the consumption of input by the throwing rule.

### Simple Match

The simple match function only takes the input as argument and has no further template parameters.

```c++
struct rule_with_simple_match
{
   // Type aliases omitted.

   template< typename ParseInput >
   static bool match( ParseInput& );
};
```

We consider it advisable to always template over the type of the input as shown above.

### Simple Example

The `even` parsing rule below shows a simple `match` function for a leaf rule using the simple interface.

```c++
struct even
{
   // Type aliases omitted.

   template< typename ParseInput >
   static bool match( ParseInput& in )
   {
      if( !in.empty() ) {
         if( ( ( *in.current() ) & 1 ) == 0 ) {
            in.template consume< even >( 1 );
            return true;
         }
      }
      return false;
   }
};
```

The first `if` checks whether the Input is not empty, i.e. whether we are not at `eof` and there is indeed a next input object to examine.
To check for at least `n` further objects in the input we would write `if( in.size( n ) >= n )`.
(See the page on [stream parsing](Stream-Parsing.md) for why `n` is passed to `in.size()`.)

If there is a next input object, the second `if` checks whether its value is even.
The function `in.current( const std::size_t n = 0 )` returns a pointer to the n-plus-first next input object.
If the rule only needs to work with characters the formulation `if( ( in.peek_char() & 1 ) == 0 )` could be used instead.

If both `if` conditions are satisfied we (a) have a next input object and (b) it is an even number.
The next step is to consume this input object from the input, which advances the pointer returned by `in.current()` and decreases the remaining size (by one).
Returning `true` concludes the successful match.

If either of the `if` conditions fails the match fails and we return `false` to signal local (match) failure.
Since no code path leading to the `return false` consumed any input it is not necessary to rewind the input to the starting point of the rule match.

In production code something like `static_assert( std::is_integral_v< typename ParseInput::data_t > )` should be added to the beginning of the function to make explicit the types of input objects on which this rule can be useful.

### Complex Match

The complex match function has additional template parameters and is called with all state arguments, the additional parameters passed to the top-level parse function.

```c++
struct rule_with_complex_match
{
   // Type aliases omitted.

   template< tao::pegtl::apply_mode A,
             tao::pegtl::rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   static bool match( ParseInput&, States&&... );
};
```

The apply mode can be `apply_mode::enabled` or `apply_mode::disabled` and determines whether action application is enabled or disabled.
Usually only the control class needs to adapt its behavior to the current apply mode.
During a parsing run the apply mode can be changed e.g. with the [`enable`](Rule-Reference.md#enable-r-) and [`disable`](Rule-Reference.md#disable-r-) rules.

The rewind mode can be `required` or `optional`.

When it is `required`, a complex match function **must not** consume input on local failure, just like all simple match functions.
When it is `optional`, this requirement is not in effect and any rewinding provisions can be optimized away.

The rewind guard explained below automatically adapts to the rewind mode and makes it easy to write efficient rules that adhere to the required protocol.

The Action and Control are required for calling sub-rules as shown in the [example below](#complex-example).

The States are also used for calling sub-rules, though it is not unheard of for parsing rules themselves to make use of them.
The two major reasons for rules to use states are (a) when a rule also does the job of an action, and (b) when a rule requires some dynamic information to determine when to match.

For an example of (a) consider the rule `from_chars_combo` in `include/tao/pegtl/extra/charconv.hpp`.
For an example of (b) consider the grammar in `src/example/dynamic_match.cpp`.

### Rewind Guard

The rewind guard is an object created by the Control that takes the rewind mode into account.

If the rewind mode is `rewind_mode::optional` the rewind guard returned by the default Control is a dummy guard that does nothing.

```c++
class dummy_guard
{
public:
   constexpr dummy_guard() noexcept = default;

   dummy_guard( dummy_guard&& ) = delete;
   dummy_guard( const dummy_guard& ) = delete;

   ~dummy_guard() = default;

   void operator=( dummy_guard&& ) = delete;
   void operator=( const dummy_guard& ) = delete;

   [[nodiscard]] constexpr bool operator()( const bool result ) const noexcept
   {
      return result;
   }
};
```

If the rewind mode is `rewind_mode::required` the rewind guard returned by the default Control will perform rewinding when required.
More precisely, it will remember the current state of the input in its constructor, and rewind the input to that state when the destructor is called and `operator()( const bool )` was **not** called with a value of `true`.

To use the rewind guard call the `guard()` function on the Control to create a local object, here `m`, and use `return m( bar )` instead of `return bar` throughout the `match()` function.

```c++
struct foo
{
   // Type aliases omitted.

   template< tao::pegtl::apply_mode A,
             tao::pegtl::rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   static bool match( ParseInput& in, States&&... st )
   {
      auto m = Control< foo >::template guard< A, M, Action, Control >( in, st... );
      // Code with multiple points that can consume
      // and/or where the decision on whether to fail
      // or succeed is made after consuming input.
      return m( result );  // Instead of return result;
   }
};
```

### Complex Example

Consider the implementation of `tao::pegtl::internal::seq` for more than one sub-rule, i.e. when `sizeof...( Rules ) > 1`.

```c++
template< typename... Rules >
struct seq
{
   // Type aliases omitted.

   template< apply_mode A,
             rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] static bool match( ParseInput& in, States&&... st )
   {
      auto m = Control< seq >::template guard< A, M, Action, Control >( in, st... );
      return m( ( Control< Rules >::template match< A, rewind_mode::optional, Action, Control >( in, st... ) && ... ) );
   }
};
```

All template parameters and function arguments provided by the complex interface are needed for the Control functions.

The rewind guard is instantiated just as in the exposition above.
The interesting part is the rewind mode passed to the sub-rules inside of the logical-and fold-expression.

If `M == rewind_mode::optional` then `seq` is allowed to consume on local failure, and therefore the sub-rules are allowed to consume on local failure.

If `M == rewind_mode::required` then the sub-rules are called "under the umbrella" of what will be a non-dummy rewind guard instantiated in the first line of the match function.

In either case the sub-rules themselves do not need to rewind on local failure, wherefore we can always invoke them with `rewind_mode::optional`.
This allows the sub-rules to optimize away their rewind guards to dummy guards.

### Type Aliases

The [grammar analysis](Debug-Facilities.md#grammar-analysis) (and some other parts) of the PEGTL require rules to define two type aliases.

The aliases `rule_t` and `subs_t` are usually defined in the classes that implement the static match function.

The alias `rule_t` is usually defined as the containing class.

The alias `subs_t` is usually defined as the meta-type-list `tao::pegtl::type_list<>` with all sub-rules as template parameters.

The type alias `tao::pegtl::empty_list` can be used as shortcut for the empty type list `tao::pegtl::type_list<>`.

The type aliases for a rule *without* sub-rules are as follows.

```c++
struct rule_without_sub_rules
{
   using rule_t = rule_without_sub_rules;
   using subs_t = tao::pegtl::empty_list;

   // Match function omitted.
};
```

The type aliases for a rule *with* sub-rules are as follows.

```c++
template< typename... Rules >
struct rule_with_sub_rules
{
   using rule_t = rule_with_sub_rules;
   using subs_t = tao::pegtl::type_list< Rules... >;

   // Match function omitted.
};
```

These type aliases are required for the meta-programming-based features of the PEGTL to recover the type and sub-rules of a rule.
The meta-data mapping part of every rule's description in the [Rule Reference](Rule-Reference.md) shows the definition of `rule_t` for all included rules.

### Exceptions

Global failures are exceptions thrown by a rule's match function.

Exceptions thrown by rules can be broadly categorized as either

1. failure to satisfy the condition for a successful match, or
2. anything else, most frequently an I/O error of a [stream input](Stream-Parsing.md).

In general, neither case requires a rule to directly throw an exception itself.

Any I/O error encountered by the reader of a [stream input](Stream-Parsing.md) will already throw an exception.

Failure to match *should* result in a local error as it *should* be the author of a grammar to decide if and where to switch to global errors with the [must-family of rules](Rule-Reference.md#exceptional).
If, however, a parsing rule must, for some reason, throw an exception itself then it *should* use the `raise` Control function which throws an exception of the appropriate type, namely `tao::pegtl::parse_error< ParseInput::error_position_t >` assuming `ParseInput` is the type of the input.

### Complete Example

The following excerpt from `src/example/dynamic_match.cpp` shows a complex custom rule that uses a state argument.

The grammar parses a *long string literal*, a string literal that can contain any character sequence with neither the necessity nor the possibility of escape sequences, as is common in many scripting languages.
The implementation here adopts the convention that a long string literal begins with `[foo[` and ends with `]foo]` where `foo` is any non-empty string that does not contain a `[`.

We could implement a single custom rule `long_string_literal` to match these strings, however then an action attached to that rule would receive the string literal *including* the delimiters as matched input.
Since the action will probably only be interested in the string literal *excluding* the delimiters we will spread things out to allow an action to attach to the content.

Please note that the following code snippets are not in actual source code order.
First we define rules for the opening sequence of a long string literal.

```c++
namespace dynamic
{
   struct long_literal_id
      : tao::pegtl::plus< tao::pegtl::not_one< '[' > > {};

   struct long_literal_open
      : tao::pegtl::seq< tao::pegtl::one< '[' >,
                         long_literal_id,
                         tao::pegtl::one< '[' > > {};
```

Then we implement an action class with a specialisation for the variable part of the long string literal opening.
The action stores that variable matched string in the State `id`.

```c++
   template< typename Rule >
   struct action {};

   template<>
   struct action< long_literal_id >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in,
                         std::string& id,
                         const std::string& )
      {
         id = in.string();
      }
   };
```

The rule for the closing sequence is similar to the opening, with closing instead of opening brackets.
This is where we require a custom rule to match the variable part to the content of the state variable `id`.

```c++
   struct long_literal_close
      : tao::pegtl::seq< tao::pegtl::one< ']' >,
                         long_literal_mark,
                         tao::pegtl::one< ']' > > {};
```

The custom rule `long_literal_mark` needs to

1. first check whether the input contains enough bytes to match `id`,
2. then check whether the input bytes match the stored string, and
3. finally call `consume()` on the input when both checks succeed.

```c++
   struct long_literal_mark
   {
      template< tao::pegtl::apply_mode A,
                tao::pegtl::rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput >
      static bool match( ParseInput& in,
                         const std::string& id,
                         const std::string& )
      {
         if( in.size( id.size() ) >= id.size() ) {
            if( std::memcmp( in.begin(), id.data(), id.size() ) == 0 ) {
               in.consume< long_literal_mark >( id.size() );
               return true;
            }
         }
         return false;
      }
   };
```

The grammar is completed with another two rules for putting everything together, and an action that stores the body of the long string literal in the state argument `body`.

```c++
   struct long_literal_body
      : tao::pegtl::any {};

   struct grammar
      : tao::pegtl::if_must< long_literal_open,
                             tao::pegtl::until< long_literal_close,
                                                long_literal_body >,
                             tao::pegtl::eof > {};

   template<> struct action< long_literal_body >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in,
                         const std::string&,
                         std::string& body )
      {
         body += in.string_view();
      }
   };

}  // namespace dynamic
```

Given the main function...

```c++
int main( int argc, char** argv )
{
   for( int i = 1; i < argc; ++i ) {
      std::string id;
      std::string body;

      tao::pegtl::argv_input< tao::pegtl::scan::lf_crlf > in( argv, i );
      if( tao::pegtl::parse< dynamic::grammar, dynamic::action >( in, id, body ) ) {
         std::cout << "long literal id was: " << id << std::endl;
         std::cout << "long literal body was: " << body << std::endl;
      }
      else {
         std::cerr << "parse error for: " << argv[ i ] << std::endl;
         return 1;
      }
   }
   return 0;
}
```

...we can see the grammar in action in the shell:

```sh
$ build/bin/example/dynamic_match '[foo["[bla]"]foo]'
long literal id was: foo
long literal body was: "[bla]"

$ build/bin/example/dynamic_match '["fraggle"["[foo["]"fraggle"]'
long literal id was: "fraggle"
long literal body was: "[foo["

$ build/bin/example/dynamic_match "[===[first line
second line]===]"
long literal id was: ===
long literal body was: first line
second line
```


## Rule Optimizations

Some notes on performance how to design grammars for best performance.

### Backtracking

For performance reasons a grammar should be designed to minimize backtracking.
We will start with a simple example.

```c++
using namespace tao::pegtl;
struct R = sor< seq< A, B >, seq< A, C > > {};  // R = (AB)/(AC)
```

If the input matches `seq< A, C >`, then matching `R` on said input will parse `A` twice (assuming that `B` does not match anything that `C` does).
The first time `A` will match successfully during the unsuccessful attempt to match `seq< A, B >`.
The second time `A` will match the same part of the input successfully again during the successful attempt to match `seq< A, C >`.
The solution is to change the grammar as follows.

```c++
struct R = seq< A, sor< B, C > > {};  // R = A(B/C)
```

Not backtracking over `A` has the additional advantage of not triggering any action attached to `A` twice.

In practice, opportunities to remove superfluous backtracking might not be as obvious as with such a simple rule.
For a more complex example please read the comments at the beginning of the [Lua 5.3 grammar](Example-Reference.md#lua53hpp) in `include/tao/pegtl/example/lua53.hpp`.
It shows how to eliminate both left-recursion and superfluous backtracking with multiple rules and recursions.

### Whitespace

Grammars should be designed to minimize redundant multiple parsing of the same whitespace, comments or other padding.

One good way to achieve this is to choose a strategy for whitespace handling and then consistently stick to it.
For example the [JSON grammar](Example-Reference.md#jsonhpp) in `include/tao/pegtl/example/json.hpp` consistently has every rule for a "token" consume any following whitespace via the `ws` rule, too.
That way every rule can assume to start matching some "real" input since any whitespace would have already been consumed by the previous one.

### Combinations

The `at<>`-rule never consumes input, and therefore always uses an input-marker to rewind the input back to where it started, regardless of the match-result.
In the context of optimizing our [JSON library](https://github.com/taocpp/json), we noticed that the combination `at< one< ... > >` could be combined into an optimized `at_one< ... >` rule:
Instead of `one< ... >` advancing the input, and `at< one< ... > >` rewinding, the combined rule would omit both the advancing and the rewinding.

Put to the test, the optimized `at_one< '"' >` rule did not show any performance advantage over `at< one< '"' > >`, at least with `-O3`.
Presumably the compiler was smart enough to perform the optimisation by itself.

However with `-O0`, the optimized `at_one< '"' >` was faster by 5-10% in a [JSON library](https://github.com/taocpp/json) micro-benchmark.
As the PEGTL should only be used with optimizations enabled, we removed the `at_one<>` rule, as we try to reduce the number of rules that don't provide a clear benefit.

We still need to test whether the compiler manages to perform the same optimisation in more complex cases.


## Rule Comparisons

The following tables compare groups of related combinators by showing their matching behaviors on some sample inputs.

A quoted string indicates that the rule matched on the input, and shows which part of the input was consumed.
The unquoted letter '**f**' stands for a "local failure" where the rule returns `false`.
The unquoted letter '**E**' stands for a "global failure" where the rule throws a `parse_error_base`-derived exception.

### Implied Preamble

```c++
using namespace tao::pegtl;

struct a : one< 'a' > {};
struct b : one< 'b' > {};
struct c : one< 'c' > {};
```

### Simple Combinators

|  | "" | "a" | "ab" | "z" | "az" | "aba" |
|--|--|--|--|--|--|--|
| [`seq< a, b >`](Rule-Reference.md#seq-r-) | **f** | **f** | "ab" | **f** | **f** | "ab" |
| [`opt< a, b >`](Rule-Reference.md#opt-r-) | "" | "" | "ab" | "" | "" | "ab" |
| [`opt_must< a, b >`](Rule-Reference.md#opt_must-r-s-) | "" | **E** | "ab" | "" | **E** | "ab" |
| [`strict< a, b >`](Rule-Reference.md#strict-r-) | "" | **f** | "ab" | "" | **f** | "ab" |
| [`partial< a, b >`](Rule-Reference.md#partial-r-) | "" | "a" | "ab" | "" | "a" | "ab" |
| [`must< a, b >`](Rule-Reference.md#must-r-) | **E** | **E** | "ab" | **E** | **E** | "ab" |
| [`if_must< a, b >`](Rule-Reference.md#if_must-r-s-) | **f** | **E** | "ab" | **f** | **E** | "ab" |

### Iterating Combinators

|  | "" | "a" | "ab" | "aba" | "abab" | "z" | "az" | "abz" | "abaz" |
|--|--|--|--|--|--|--|--|--|--|
| [`plus< a, b >`](Rule-Reference.md#plus-r-) | **f** | **f** | "ab" | "ab" | "abab" | **f** | **f** | "ab" | "ab" |
| [`star< a, b >`](Rule-Reference.md#star-r-) | "" | "" | "ab" | "ab" | "abab" | "" | "" | "ab" | "ab" |
| [`star_strict< a, b >`](Rule-Reference.md#star_strict-r-) | "" | **f** | "ab" | **f** | "abab" | "" | **f** | "ab" | **f** |
| [`star_partial< a, b >`](Rule-Reference.md#star_partial-r-) | "" | "a" | "ab" | "aba" | "abab" | "" | "a" | "ab" | "aba" |
| [`star_must< a, b >`](Rule-Reference.md#star_must-r-s-) | "" | **E** | "ab" | **E** | "abab" | "" | **E** | "ab" | **E** |

### Repeating Combinators

|  | "" | "a" | "aa" | "aaa" | "aaaa" |
|--|--|--|--|--|--|
| [`rep< 2, a >`](Rule-Reference.md#rep-num-r-) | **f** | **f** | "aa" | "aa" | "aa" |
| [`rep_opt< 2, a >`](Rule-Reference.md#rep_opt-num-r-) | "" | "a" | "aa" | "aa" | "aa" |
| [`rep_min< 2, a >`](Rule-Reference.md#rep_min-min-r-) | **f** | **f** | "aa" | "aaa" | "aaaa" |
| [`rep_max< 2, a >`](Rule-Reference.md#rep_max-max-r-) | "" | "a" | "aa" | **f** | **f** |
| [`rep_min_max< 2, 3, a >`](Rule-Reference.md#rep_min_max-min-max-r-) | **f** | **f** | "aa" | "aaa" | **f** |

### List Combinators

|  | "" | "a" | "aa" | "ab" | "aba" | "abab" | "abc" | "ac" | "acb" | "acba" | "acbca" |
|--|--|--|--|--|--|--|--|--|--|--|--|
| [`list< a, b >`](Rule-Reference.md#list-r-s-) | f | "a" | "a" | "a" | "aba" | "aba" | "a" | "a" | "a" | "a" | "a" |
| [`list_opt< a, b >`](Rule-Reference.md#list_opt-r-s-) | "" | "a" | "a" | "a" | "aba" | "aba" | "a" | "a" | "a" | "a" | "a" |
| [`list_tail< a, b >`](Rule-Reference.md#list_tail-r-s-) | f | "a" | "a" | "ab" | "aba" | "abab" | "ab" | "a" | "a" | "a" | "a" |
| [`list_must< a, b >`](Rule-Reference.md#list_must-r-s-) | f | "a" | "a" | E | "aba" | E | E | "a" | "a" | "a" | "a" |
| [`list< a, b, c >`](Rule-Reference.md#list-r-s-p-) | f | "a" | "a" | "a" | "aba" | "aba" | "a" | "a" | "a" | "acba" | "acbca" |
| [`list_opt< a, b, c >`](Rule-Reference.md#list_opt-r-s-p-) | "" | "a" | "a" | "a" | "aba" | "aba" | "a" | "a" | "a" | "acba" | "acbca" |
| [`list_tail< a, b, c >`](Rule-Reference.md#list_tail-r-s-p-) | f | "a" | "a" | "ab" | "aba" | "abab" | "ab" | "a" | "acb" | "acba" | "acbca" |
| [`list_must< a, b, c >`](Rule-Reference.md#list_must-r-s-p-) | f | "a" | "a" | E | "aba" | E | E | "a" | E | "acba" | "acbca" |

### ASCII String Rules

|  | "" | "inf" | "infinityx" | "iNFi" | "infiNiTy" |
|--|--|--|--|--|--|
| [`string< "infinity" >`](Rule-Reference.md#string-text-) | f | f | "infinity" | f | f |
| [`istring< "infinity" >`](Rule-Reference.md#istring-text-) | f | f | "infinity" | f | "infiNiTy" |
| [`astring< 3, "infinity" >`](Rule-Reference.md#astring-p-text-) | f | "inf" | "infinity" | f | "inf" |
| [`aistring< 3, "infinity" >`](Rule-Reference.md#aistring-p-text-) | f | "inf" | "infinity" | "iNF" | "infiNiTy" |

These rules take string literals directly as template arguments.
The [character-pack forms](Rule-Reference.md#character-packs) are available under names ending in `_chars`.

### ASCII Matching Rules

|  | "a" | "c" | "e" | "G" | "Z" | "\xA4" |
|--|--|--|--|--|--|--|
| [`any`](Rule-Reference.md#any) | "a" | "c" | "e" | "G" | "Z" | "\xA4" |
| [`one< 'c', 'g' >`](Rule-Reference.md#one-c-) | **f** | "c" | **f** | **f** | **f** | **f** |
| [`ione< 'c', 'g' >`](Rule-Reference.md#ione-c-) | **f** | "c" | **f** | "G" | **f** | **f** |
| [`range< 'c', 'g' >`](Rule-Reference.md#range-c-d-) | **f** | "c" | "e" | **f** | **f** | **f** |
| [`not_one< 'c', 'g' >`](Rule-Reference.md#not_one-c-) | "a" | **f** | "e" | "G" | "Z" | "\xA4" |
| [`not_ione< 'c', 'g' >`](Rule-Reference.md#not_ione-c-) | "a" | **f** | "e" | **f** | "Z" | "\xA4" |
| [`not_range< 'c', 'g' >`](Rule-Reference.md#not_range-c-d-) | "a" | **f** | **f** | "G" | "Z" | "\xA4" |
| [`any7`](Rule-Reference.md#any7) | "a" | "c" | "e" | "G" | "Z" | **f** |
| [`not_one7< 'c', 'g' >`](Rule-Reference.md#not_one7-c-) | "a" | **f** | "e" | "G" | "Z" | **f** |
| [`not_ione7< 'c', 'g' >`](Rule-Reference.md#not_ione7-c-) | "a" | **f** | "e" | **f** | "Z" | **f** |
| [`not_range7< 'c', 'g' >`](Rule-Reference.md#not_range7-c-d-) | "a" | **f** | **f** | "G" | "Z" | **f** |


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
