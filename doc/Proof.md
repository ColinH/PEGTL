# Completeness of the Grammar Analysis

## Result

The cycle search in
[`analyze_cycles.hpp`](../include/tao/pegtl/debug/internal/analyze_cycles.hpp)
is complete for the visible, pure grammar described by the restrictions below.

The claim cannot cover literally every rule shipped with the PEGTL.
Rules such as `source` deliberately hide a nested grammar, `unordered` has a
documented inexact abstraction, some rules deliberately reject analysis, and
arbitrary callbacks can perform arbitrary C++.  These cases are excluded from
the theorem rather than weakening the result for ordinary visible grammars.

Informally, the theorem applies to pure grammars for which:

1. every recursive parse is visible in the `analyze_traits` graph;
2. the rules classified as consuming really consume on every successful
   match;
3. rules with arbitrary, deliberately invisible, or incorrectly abstracted
   behavior are excluded.

Under these restrictions, if parsing can loop forever without progress, the
analysis reports at least one problem.  A precise statement and a proof are
given below.

## What is being proved

This document concerns *grammar-induced non-termination without progress*.
It does not attempt to detect arbitrary non-termination in an action, a
control hook, an input implementation, a state object, a semantic function,
or user code called by any of them.

For an ordinary PEGTL input, let its position be its logical offset in the
input.  A **zero-progress loop** is an infinite parser execution with an
infinite chain of activations of some rule or unbounded repetition at the same
logical offset, with no *committed net* input advance between consecutive
activations in that chain.  Speculative matches may temporarily inspect or
consume later input and then be rewound by lookahead or backtracking.  A
`rematch` view uses the logical offset in the underlying matched span rather
than the identity of its temporary input object.

This definition covers both important implementation shapes:

* unbounded recursive descent, including direct and indirect left recursion;
* an unbounded loop inside `star`, `plus`, `star_partial`, `star_strict`, or
  `until`, whose body repeatedly succeeds without consuming input.

It does not call parsing an infinite stream a zero-progress loop merely
because a rule can keep consuming that stream forever.

## The abstract grammar built by `analyze_traits`

### Entries

Starting with the grammar `G`, `analyze_insert` recursively constructs a
finite map of entries.  Write its set of entry names as $V$.  An entry
$v\in V$ has:

* a kind $k(v)\in\{\mathsf{any},\mathsf{opt},\mathsf{seq},\mathsf{sor}\}$;
* an ordered list of children
  $\operatorname{ch}(v)=(v_1,\ldots,v_n)$.

The kind and children come from
`analyze_traits< Name, typename Name::rule_t >`.  Insertion before recursive
insertion of the children makes a recursive reference finite: a second
encounter with the same rule name reuses its existing entry.

We assume here and in the theorem that trait expansion terminates, that $V$
is finite, and that the demangled strings used as map keys distinguish the
rule types in $V$.

### The consuming Boolean

The return value of `work` represents whether the analysis can prove that a
rule consumes whenever it succeeds.  It is useful to describe this value as
the least solution $C:V\to\{0,1\}$ of these monotone Boolean equations:

\[
\begin{aligned}
C(v) &= 1
  &&\text{if } k(v)=\mathsf{any},\\
C(v) &= 0
  &&\text{if } k(v)=\mathsf{opt},\\
C(v) &= \bigvee_{i=1}^{n} C(v_i)
  &&\text{if } k(v)=\mathsf{seq},\\
C(v) &= \bigwedge_{i=1}^{n} C(v_i)
  &&\text{if } k(v)=\mathsf{sor}.
\end{aligned}
\]

Thus `any` means "consuming by itself", not "match any character".  Likewise,
`opt` means "not proved consuming by itself"; it does not necessarily denote
the public `opt` combinator.

The equations only establish the one-way property

> $C(v)=1$ implies that every successful terminating match of $v$
> consumes input.

The converse is neither needed nor generally true.  In particular,
$C(v)=0$ does not prove that $v$ can succeed without consuming.  This
one-way abstraction is an important source of false positives.

### Why `work` computes the least solution

The following argument concerns the Boolean returned by `work`; its reporting
side effect is considered in the next section.

1. The four equations are monotone on the finite lattice
   $\{0,1\}^{V}$, so they have a least fixed point.
2. A value $C(v)=1$ has a finite proof tree.  An `any` node is an axiom, a
   `seq` node chooses one true child, and a `sor` node proves every child.
   An `opt` node has no proof.
3. Choose a smallest such proof tree.  No rule name repeats on an
   ancestor--descendant path.  If it did, the subtree at the lower occurrence
   would be a strictly smaller proof of the same rule and could replace the
   upper occurrence.
4. `work` searches exactly these finite proof trees.  Its OR loops implement
   the choice made by a `seq` proof, its AND loop implements all premises of a
   `sor` proof, and `any` and `opt` return the two constants.
5. When `work` encounters a name already on its recursion stack, it returns
   false.  By step 3 this cannot remove every proof of a true least-fixed-point
   value.  Conversely, every true return constructs a finite proof tree.

Therefore the root result stored in `m_results` is precisely $C(v)$.

This characterization is mathematical scaffolding for the proof; the
implementation evaluates it by repeated depth-first traversals rather than by
an explicit fixed-point iteration.

## The graph actually searched for cycles

The analyzer does not traverse every child of every `any`, `opt`, or `seq`
entry.  C++ short-circuit evaluation in their assignments inside `work` prunes
the traversal after a child proved consuming.  In contrast, every child of a
`sor` entry is evaluated: `work(...)` is the left operand of `&&`, while the
accumulated Boolean is the right operand.

Define the **searched graph** $D_C=(V,E_C)$ as follows.  For an entry with
children $v_1,\ldots,v_n$:

* if its kind is `any`, `opt`, or `seq`, put $v\to v_i$ in $E_C$ exactly
  when

  \[
  C(v_1)=\cdots=C(v_{i-1})=0;
  \]

* if its kind is `sor`, put $v\to v_i$ in $E_C$ for every child.

The empty prefix satisfies either condition, so every non-leaf entry has an
edge to its first child.

For OR-scanned kinds, traversal stops after the first child proved consuming.
For `sor`, traversal and Boolean aggregation are separate: every child is
searched, while the returned consumption value remains the conjunction of all
child values.

### The `accum` parameter is always false on an executed call

This implementation detail simplifies the cycle argument.

1. Every top-level call from `problems()` is `work(entry, false)`.
2. A `sor` child receives `accum` unchanged.
3. In the other three cases, the child expression is evaluated only while the
   local OR accumulator `a` is false.  Hence the argument `accum || a` equals
   `accum` whenever the call is actually made.
4. Induction on call depth shows that every executed call receives false.

Consequently every encounter with a name already on `m_stack` increments
`m_problems`; no encountered back-edge is suppressed by `accum`.

### Cycle-detection lemma

**Lemma.** If $D_C$ contains a directed cycle, `problems()` returns a
non-zero value.

**Proof.** `problems()` starts a traversal separately at every entry, so start
at a vertex on the cycle.  At an OR-scanned vertex, all children preceding the
next cycle edge have $C=0$, and their calls therefore return false unless they
already encounter and report a back-edge.  Indeed, a contextual true return
would itself be a finite proof tree and would imply $C=1$.  The traversal
consequently reaches the cycle edge.  At a `sor` vertex, every child is called
unconditionally, so the traversal also reaches the cycle edge regardless of
the preceding child values.  Thus either a problem is reported early or
traversal continues along the cycle.  Because the cycle is finite, the latter
case eventually encounters its starting vertex on `m_stack`.  The `accum`
lemma then shows that `m_problems` is incremented.  ∎

Only this direction is needed.  The converse need not correspond to a real
parser loop because the abstract graph deliberately over-approximates many
semantic facts.

## Obstacles outside the main theorem

The following non-extra PEGTL facilities remain outside the theorem.

| Rule or facility | Obstacle |
| --- | --- |
| `source< R >` | Its traits are `analyze_opt_traits<>` and do not expose `R`.  For example, `source< star< success > >` analyzes as an acyclic leaf although its nested parse loops. |
| `nested< R, P >` | `internal::nested` has empty `subs_t` and no specialization in `debug/analyze_traits.hpp`; attempting to analyze it is not supported. |
| `unordered< R... >`, `unordered_partial< R... >` | Their traits model the children as a sequence, although the implementation tries them as changing ordered choices.  `struct bad : unordered< one< 'a' >, bad > {};` loops on `"b"` but analyzes as zero problems.  The rule reference already documents this limitation. |
| `function` and `restart` | Their trait specializations deliberately fail with a `static_assert`; they are not analyzable. |
| Actions, controls, states, custom peeks, custom EOL rules, inputs, and true custom rules | They can run arbitrary code or invalidate the consumption and child-visibility contracts. |

## Restricted completeness theorem

### Assumptions

Let `G` be a grammar and let $V$, $C$, and $D_C$ be defined as above.
Assume all of the following.

**A1. Finite and pure execution.** Trait expansion produces a finite graph.
All actions, control hooks, state constructors and callbacks, input methods,
peek methods, EOL methods, exception machinery, and other invoked user code
terminate.  They do not recursively invoke parsing, move the input position,
or alter which grammar rules are matched.  Built-in actions that merely
observe or update semantic state are allowed.

**A2. Ordinary input movement.** Excluding explicit lookahead and documented
nested/rematch views, a successful rule never moves the logical input offset
backwards.  A standard positive-width peek has positive size.  A built-in EOL
rule consumes at least one input unit on success.  Failed alternatives rewind
as required by the normal PEGTL combinator semantics.

**A3. Sound consuming classifications.** For every $v\in V$, if $C(v)=1$,
then every successful terminating match represented by $v$ advances the
relevant logical input offset by at least one unit.  This is the one-way trait
contract stated in `analyze_traits.hpp`.

**A4. Complete visibility.** Every grammar-rule invocation that can take part
in a zero-progress loop is either a child in the traits graph or is represented
by an explicit recursive edge introduced by a trait transformation.  In
particular, the grammar contains no invisible nested grammar such as
`source`, `nested`, or a custom rule that calls an unlisted rule.

**A5. Supported control flow.** The grammar does not contain `restart`,
`function`, `unordered`, or `unordered_partial`.  A true custom rule is
allowed only if its trait kind, ordered children, consumption behavior, and
bounded/unbounded repetition behavior satisfy the same contracts used in this
proof.

### Scope of the rule audit

Subject to A1--A5, the standard traits support the following families.

| Abstract treatment | Representative rules | Reason it preserves the proof obligation |
| --- | --- | --- |
| Positive-width or vacuously consuming leaf (`any`) | `any`, `one`, `ione`, `range`, `ranges`, non-empty strings, positive `consume`/`many`, `eol`, `failure`, `raise` | On success the ordinary terminal consumes; a never-successful or always-throwing leaf satisfies consumption-on-success vacuously. |
| Possibly empty leaf or wrapper (`opt`) | `success`, `bof`, `bol`, `eof`, `eolf`, `everything`, zero `consume`/`many`, `at`, `not_at`, `opt`, `partial`, `strict`, `apply`, `apply0`, and terminating stream-management leaves | They are never used as evidence of mandatory consumption.  Their listed children are still searched until a consuming barrier. |
| Ordered conjunction (`seq`) | `seq`, positive bounded repetition, `must`, non-optional `if_must`, action/control/enable/disable/state wrappers, and exception wrappers | A same-position call of child $i$ can follow only children that succeeded without mandatory consumption. |
| Ordered choice (`sor`) | `sor` and transformations of `if_then_else` and `rematch` | Every alternative is searched unconditionally; the conjunction of child values is used only for the consumption result. |
| Explicit unbounded repetition | `star`, `star_partial`, `star_strict`, `plus`, `until`, and aliases such as `list`, `pad`, and `rep_min` | Their transformations insert the repeated rule name, or reduce to a `star` containing it, after the body.  An empty successful body therefore creates a searched self-cycle. |
| Finite repetition | `rep`, `rep_min_max`, `rep_opt` | A finite loop cannot itself cause non-termination; a loop inside its body remains visible as a child loop. |

ASCII, UTF, Unicode, member, and deprecated aliases are covered when their
`rule_t` reduces to the audited built-in leaf/combinator and their peek or
action behavior satisfies A1--A3.  The stream rules `discard`, `is_stream`,
`prefetch`, and `require` are harmless zero-width leaves only when their input
operations terminate.  Extras and examples are outside the requested scope.

### Theorem

**Theorem (restricted completeness).** Under assumptions A1--A5, if parsing
`G` has a grammar-induced zero-progress loop, then

```c++
analyze< G >( -1 ) != 0
```

That is, within this scope the analysis has no false negatives.  It can still
have false positives.

### Proof

The proof is by converting an infinite runtime execution into an infinite path
in $D_C$, then using finiteness and the cycle-detection lemma.

#### Step 1: Locate the source of unbounded execution

Every ordinary terminal, wrapper operation, finite repetition, and visit to a
finite child list terminates by A1.  Therefore an infinite execution has at
least one of these forms after a finite prefix:

1. the C++ match-call stack grows without bound; or
2. a bounded part of that stack contains an unbounded PEGTL repetition that
   iterates without bound.

Finite ordered choice and finite backtracking cannot constitute a third case:
they contain finitely many alternatives and retry an alternative only when an
enclosing unbounded construct invokes them again.

#### Step 2: Choose the zero-net-progress dependency chain

By the definition of a zero-progress loop, discard a finite execution prefix
and choose the infinite chain of recurrent activations at one logical offset.
Between two chain elements, record the abstract entries on the non-returning
recursive descent, or record the enclosing unbounded repetition whose body
returns successfully and begins again with no net advance.

A2 excludes a hidden backward jump by a successful ordinary rule.  Lookahead
can restore its own entry position, but the lookahead wrapper is classified as
non-consuming.  If a speculative child of lookahead returns, control returns
to that wrapper before the zero-position chain continues; if it does not
return, the infinite suffix is inside the child and ordinary non-returning
descent applies.  A rematch view uses the logical offset of the outer matched
span.  Its traits expose the head and each secondary rule through synthetic
`sor` alternatives; exhaustive choice traversal searches all of them,
including when the head can match without consuming.

A4 ensures that none of the activations selected for this dependency chain
disappears from the abstract grammar.

#### Step 3: A conjunction transition is an edge of $D_C$

Suppose the selected zero-position dependency chain reaches child $v_i$ of
an `any`, `opt`, or `seq`-scanned entry $v$.  Every preceding child that had
to finish before $v_i$ was entered finished successfully without a
committed move of this dependency level's position.  If some preceding child
$v_j$ had $C(v_j)=1$, A3 would force that success to consume input, and A2
would prevent the later ordinary children from undoing it.  The dependency
could not return to the same offset through $v_i$, a contradiction.  Hence

\[
C(v_1)=\cdots=C(v_{i-1})=0.
\]

By definition, $v\to v_i$ is an edge of $D_C$.

This argument also applies inside `at`, `not_at`, `partial`, and `strict`:
although the enclosing wrapper has special success or rewind behavior, its
children are attempted in conjunction order, and a non-returning child is
reached before the wrapper can perform its final rewind or result conversion.

#### Step 4: An ordered-choice transition is an edge of $D_C$

Suppose execution selects child $v_i$ of a transformed `sor` entry.  The
earlier alternatives returned failure and were rewound.  Runtime failure does
not imply anything about $C$, which concerns success.  The analyzer no longer
uses that unrelated Boolean to prune choice traversal: by definition of
$D_C$, $v\to v_i$ is an edge for every `sor` child.  Thus every runtime
ordered-choice transition is represented, without assumptions about the
earlier alternatives.

#### Step 5: An unbounded iteration gives the transformed self-edge path

Consider an unbounded repetition that starts another body match at the same
position.

* The previous body match succeeded without consuming.  By A3, no mandatory
  conjunction prefix on the path through that body has $C=1$.
* `star`, `star_partial`, and `star_strict` use traits equivalent for this
  purpose to `opt< Body..., Name >`.
* `plus` uses traits equivalent to
  `seq< Body..., opt< Name > >`.
* `until< Cond, Body... >` reduces to a sequence containing a transformed
  `star< Body... >`; `until< Cond >` consumes one input unit whenever its
  condition fails, so only a loop inside `Cond` can be a fixed-position loop.

By repeated use of step 3, the abstract path reaches the inserted `Name` and
therefore represents the next iteration as a return to the repetition entry.
If instead execution is already non-terminating inside the condition or body,
steps 3 and 4 continue recording that internal path.

Bounded repetitions need no self-edge: they execute only finitely many body
matches, so non-termination must already occur inside a visible child.

#### Step 6: Obtain an infinite path

Apply steps 3--5 to every transition in the selected zero-net-progress
dependency chain.  Terminals cannot end this construction because they
terminate and have no children.  Thus the recorded entries form an infinite
directed path

\[
v_0\to v_1\to v_2\to\cdots
\]

in $D_C$.

This covers both cases from step 1.  In the unbounded-stack case the edges
come from nested calls.  In the bounded-stack case, step 5 supplies an edge
path for each new repetition.

#### Step 7: Finiteness yields a cycle

The vertex set $V$ is finite by A1.  An infinite path in a finite directed
graph repeats a vertex.  The segment between two occurrences of that vertex
is a directed cycle in $D_C$.

#### Step 8: The analyzer reports the cycle

By the cycle-detection lemma, a directed cycle in $D_C$ causes `work` to
encounter an entry already on `m_stack`.  The `accum` lemma shows that this
increments `m_problems`.  Hence `analyze< G >( -1 )` returns a non-zero value.
This proves the theorem.  ∎

## Why false positives remain possible

The theorem proves only completeness in the restricted domain, not soundness.
The searched graph can contain infeasible paths because the abstraction does
not track:

* whether a rule can actually succeed or fail on the same input;
* correlations between predicates and later alternatives;
* the concrete character or object at the current position;
* semantic state, actions, and control state;
* mutually incompatible conditions on different visits to a rule.

For example, the existing test grammar

```c++
struct bad : seq< eof, at< digit >, bad > {};
```

is reported even though `eof` and `at< digit >` cannot both succeed at the
same position.  Such over-approximation is compatible with the restricted
completeness theorem.

---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL)
documentation.
