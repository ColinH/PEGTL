// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXPERIMENT_EXPRESSION_HPP
#define TAO_PEGTL_EXPERIMENT_EXPRESSION_HPP

#if !defined( __cpp_exceptions )
#error "Exception support required for tao/pegtl/experiment/expression.hpp"
#endif

#include <cassert>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../match.hpp"
#include "../nothing.hpp"
#include "../rewind_mode.hpp"
#include "../rules.hpp"
#include "../type_list.hpp"

#include "../action/builders.hpp"
#include "../action/internal/single_rule_action.hpp"
#include "../action/require_apply0.hpp"
#include "../extra/unescape.hpp"
#include "../internal/dependent_false.hpp"

namespace TAO_PEGTL_NAMESPACE::experimental
{
   enum class placement : bool
   {
      prefix,
      postfix
   };

   // Builders produce atom_t< Rule >; consumers map atoms and form
   // applications to value_t. A consumer can declare context_t to receive
   // the corresponding parse state as the first callback argument. Groups
   // are transparent and do not invoke the consumer.

   template< typename T >
   struct consumer_base
   {
      using value_t = T;

      template< typename Rule >
      using atom_t = T;

      template< typename Rule, typename Object >
      [[nodiscard]] static T atom( Object&& object )
      {
         return T( std::forward< Object >( object ) );
      }

      template< typename Rule, typename Context, typename Object >
      [[nodiscard]] static T atom( Context& /*unused*/, Object&& object )
      {
         return atom< Rule >( std::forward< Object >( object ) );
      }
   };

   namespace internal
   {
      // Recognition only needs the initial/trailing syntax. Binding powers are
      // intentionally absent; they affect construction, not accepted input.

      template< typename Expression >
      struct recognize_expression;

      template< typename Expression >
      struct recognize_initial;

      template< typename Expression >
      using recognize_skip = star< typename Expression::ignored_t >;

      template< typename Rules >
      struct atom_rule
      {
         static_assert( TAO_PEGTL_NAMESPACE::internal::dependent_false< Rules >, "an atom builder must have exactly one boundary rule" );
      };

      template< typename Rule >
      struct atom_rule< type_list< Rule > >
      {
         using type = Rule;
      };

      template< typename Binding >
      using atom_binding_t = TAO_PEGTL_NAMESPACE::internal::as_binding_t< Binding >;

      template< typename Binding >
      using atom_rule_t = atom_rule< typename atom_binding_t< Binding >::rules_t >::type;

      template< typename T >
      [[nodiscard]] T&& only( std::vector< T >& values ) noexcept
      {
         assert( values.size() == 1 );
         return std::move( values.front() );
      }

      template< unsigned BindingPower >
      consteval unsigned next_binding_power()
      {
         static_assert( BindingPower < static_cast< unsigned >( -1 ), "left-associative binding power must be less than the maximum unsigned value" );
         return BindingPower + 1;
      }

      template< typename Expression >
      struct recognition_context
      {
         using skip = recognize_skip< Expression >;

         template< typename, typename Grammar >
         using initial = Grammar;

         template< typename, typename Grammar >
         using trailing = Grammar;

         template< unsigned >
         using operand = recognize_initial< Expression >;

         template< unsigned >
         using nested = recognize_expression< Expression >;

         template< typename Binding >
         using atom = atom_rule_t< Binding >;
      };

      template< typename Parser, typename Rule >
      struct atom_output
      {
         using value_t = Parser::consumer_t::template atom_t< Rule >;
         using result_t = Parser::value_t;

         explicit atom_output( Parser& parser, std::optional< result_t >& result )
            : parser( parser ),
              result( result )
         {}

         template< typename Object >
         void store( Object&& object )
         {
            assert( !result );
            result.emplace( parser.template atom< Rule >( std::forward< Object >( object ) ) );
         }

         Parser& parser;
         std::optional< result_t >& result;
      };

      template< typename Form, typename Grammar >
      struct initial_form_rule
      {
         using rule_t = initial_form_rule;
         using subs_t = type_list< Grammar >;

         template< apply_mode A,
                   rewind_mode M,
                   template< typename... > class Action,
                   template< typename... > class Control,
                   typename ParseInput,
                   typename Parser,
                   typename Captured,
                   typename Result,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, Parser& parser, Captured& captured, Result& result, States&&... states )
         {
            const bool matched = TAO_PEGTL_NAMESPACE::match< Grammar, A, M, Action, Control >( in, parser, captured, result, states... );
            if( !matched ) {
               captured.clear();
            }
            return matched;
         }
      };

      template< typename Form, typename Grammar >
      struct trailing_form_rule
      {
         using rule_t = trailing_form_rule;
         using subs_t = type_list< Grammar >;

         template< apply_mode A,
                   rewind_mode M,
                   template< typename... > class Action,
                   template< typename... > class Control,
                   typename ParseInput,
                   typename Parser,
                   typename Captured,
                   typename Value,
                   typename... States >
         [[nodiscard]] static bool match( ParseInput& in, Parser& parser, Captured& captured, Value& value, const unsigned minimum, States&&... states )
         {
            if( Form::left < minimum ) {
               return false;
            }
            const bool matched = TAO_PEGTL_NAMESPACE::match< Grammar, A, M, Action, Control >( in, parser, captured, value, minimum, states... );
            if( !matched ) {
               captured.clear();
            }
            return matched;
         }
      };

      template< unsigned BindingPower >
      struct expression_source
      {
         static constexpr const char* error_message = "expected expression operand";

         template< typename Parser >
         [[nodiscard]] static std::optional< typename Parser::value_t > get( Parser& parser )
         {
            return parser.parse_expression( BindingPower );
         }
      };

      template< typename Binding >
      struct atom_source
      {
         static constexpr const char* error_message = "expected expression atom";

         template< typename Parser >
         [[nodiscard]] static std::optional< typename Parser::value_t > get( Parser& parser )
         {
            return parser.template parse_atom< Binding >();
         }
      };

      template< typename Source >
      struct captured_operand
         : success
      {
         static constexpr const char* error_message = Source::error_message;
      };

      template< typename Parser >
      struct construction_context
      {
         using skip = star< typename Parser::ignored_t >;

         template< typename Form, typename Grammar >
         using initial = initial_form_rule< Form, Grammar >;

         template< typename Form, typename Grammar >
         using trailing = trailing_form_rule< Form, Grammar >;

         template< unsigned BindingPower >
         using operand = captured_operand< expression_source< BindingPower > >;

         template< unsigned BindingPower >
         using nested = operand< BindingPower >;

         template< typename Binding >
         using atom = captured_operand< atom_source< Binding > >;
      };

      struct form_base
      {
         template< typename >
         using initial = failure;

         template< typename >
         using trailing = failure;
      };
   }  // namespace internal

   // Forms keep their recognition grammar, construction action, and
   // binding-power metadata together. A custom form must commit before it
   // captures operands because consumer side effects are not rewound when
   // another alternative is tried.

   template< typename Binding >
   struct atom
      : internal::form_base
   {
      using rule_t = internal::atom_rule_t< Binding >;

      template< typename Context >
      using grammar = Context::template atom< Binding >;

      template< typename Context >
      using initial = Context::template initial< atom, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& /*unused*/, std::optional< typename Parser::value_t >& result, std::vector< typename Parser::value_t >& captured )
      {
         result.emplace( internal::only( captured ) );
      }
   };

   template< typename Rule,
             template< typename... > class Unescape = unescape,
             typename Content = Rule::content >
   using unescape_atom = atom< multi_for< Rule, create_for< Content, std::string, Unescape > > >;

   template< typename Open, typename Close >
   struct group
      : internal::form_base
   {
      template< typename Context >
      using grammar = if_must< Open,
                               typename Context::skip,
                               typename Context::template nested< 0 >,
                               Close >;

      template< typename Context >
      using initial = Context::template initial< group, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& /*unused*/, std::optional< typename Parser::value_t >& result, std::vector< typename Parser::value_t >& captured )
      {
         result.emplace( internal::only( captured ) );
      }
   };

   template< typename Rule, unsigned BindingPower >
   struct prefix
      : internal::form_base
   {
      using rule_t = Rule;

      static constexpr placement position = placement::prefix;
      static constexpr unsigned right = BindingPower;

      template< typename Context >
      using grammar = if_must< Rule,
                               typename Context::skip,
                               typename Context::template operand< right > >;

      template< typename Context >
      using initial = Context::template initial< prefix, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, std::optional< typename Parser::value_t >& result, std::vector< typename Parser::value_t >& captured )
      {
         result.emplace( parser.template apply< prefix >( internal::only( captured ) ) );
      }
   };

   template< typename Rule, unsigned BindingPower >
   struct postfix
      : internal::form_base
   {
      using rule_t = Rule;

      static constexpr placement position = placement::postfix;
      static constexpr unsigned left = BindingPower;

      template< typename >
      using grammar = Rule;

      template< typename Context >
      using trailing = Context::template trailing< postfix, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, Parser::value_t& value, [[maybe_unused]] std::vector< typename Parser::value_t >& captured )
      {
         assert( captured.empty() );
         value = parser.template apply< postfix >( std::move( value ) );
      }
   };

   template< typename Rule, unsigned Left, unsigned Right >
   struct infix
      : internal::form_base
   {
      using rule_t = Rule;

      static constexpr unsigned left = Left;
      static constexpr unsigned right = Right;

      template< typename Context >
      using grammar = if_must< Rule,
                               typename Context::skip,
                               typename Context::template operand< right > >;

      template< typename Context >
      using trailing = Context::template trailing< infix, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, Parser::value_t& value, std::vector< typename Parser::value_t >& captured )
      {
         value = parser.template apply< infix >( std::move( value ), internal::only( captured ) );
      }
   };

   template< typename Open, typename Separator, typename Close, unsigned BindingPower >
   struct application
      : internal::form_base
   {
      using open_rule_t = Open;
      using separator_rule_t = Separator;
      using close_rule_t = Close;

      static constexpr unsigned left = BindingPower;

      template< typename Context >
      using grammar = if_must< Open,
                               typename Context::skip,
                               opt< list< typename Context::template nested< 0 >,
                                          seq< Separator, typename Context::skip > > >,
                               Close >;

      template< typename Context >
      using trailing = Context::template trailing< application, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, Parser::value_t& value, std::vector< typename Parser::value_t >& captured )
      {
         value = parser.template apply< application >( std::move( value ), std::move( captured ) );
      }
   };

   template< typename Operator, typename Name, unsigned BindingPower >
   struct member
      : internal::form_base
   {
      using rule_t = Operator;

      static constexpr unsigned left = BindingPower;

      template< typename Context >
      using grammar = if_must< Operator,
                               typename Context::skip,
                               typename Context::template atom< Name > >;

      template< typename Context >
      using trailing = Context::template trailing< member, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, Parser::value_t& value, std::vector< typename Parser::value_t >& captured )
      {
         value = parser.template apply< member >( std::move( value ), internal::only( captured ) );
      }
   };

   template< typename First, typename Second, unsigned BindingPower >
   struct ternary
      : internal::form_base
   {
      using first_rule_t = First;
      using second_rule_t = Second;

      static constexpr unsigned left = BindingPower;
      static constexpr unsigned right = BindingPower;

      template< typename Context >
      using grammar = if_must< First,
                               typename Context::skip,
                               typename Context::template nested< 0 >,
                               Second,
                               typename Context::skip,
                               typename Context::template nested< right > >;

      template< typename Context >
      using trailing = Context::template trailing< ternary, grammar< Context > >;

      template< typename Parser >
      static void build( Parser& parser, Parser::value_t& value, std::vector< typename Parser::value_t >& captured )
      {
         assert( captured.size() == 2 );
         value = parser.template apply< ternary >( std::move( value ), std::move( captured[ 0 ] ), std::move( captured[ 1 ] ) );
      }
   };

   template< typename Rule, unsigned BindingPower >
   using infix_left = infix< Rule, BindingPower, internal::next_binding_power< BindingPower >() >;

   template< typename Rule, unsigned BindingPower >
   using infix_right = infix< Rule, BindingPower, BindingPower >;

   template< typename... Parts >
   struct forms
   {
      static_assert( sizeof...( Parts ) > 0 );

      template< typename Context >
      using initial = sor< typename Parts::template initial< Context >... >;

      template< typename Context >
      using trailing = sor< typename Parts::template trailing< Context >... >;
   };

   namespace internal
   {
      template< typename Expression >
      struct recognize_initial
         : Expression::forms_t::template initial< recognition_context< Expression > >
      {};

      template< typename Expression >
      struct recognize_expression
         : seq< recognize_initial< Expression >,
                recognize_skip< Expression >,
                star< typename Expression::forms_t::template trailing< recognition_context< Expression > >,
                      recognize_skip< Expression > > >
      {};

      template< typename Rule >
      struct construction_action
         : nothing< Rule >
      {};

      template< typename Source >
      struct construction_action< captured_operand< Source > >
         : require_apply0
      {
         template< typename Parser, typename Captured, typename... States >
         [[nodiscard]] static bool apply0( Parser& parser, Captured& captured, States&&... /*unused*/ )
         {
            if( auto result = Source::get( parser ) ) {
               captured.emplace_back( std::move( *result ) );
               return true;
            }
            return false;
         }
      };

      template< typename Form, typename Grammar >
      struct construction_action< initial_form_rule< Form, Grammar > >
         : require_apply0
      {
         template< typename Parser, typename... States >
         static void apply0( Parser& parser, std::vector< typename Parser::value_t >& captured, std::optional< typename Parser::value_t >& result, States&&... /*unused*/ )
         {
            Form::build( parser, result, captured );
            assert( result );
         }
      };

      template< typename Form, typename Grammar >
      struct construction_action< trailing_form_rule< Form, Grammar > >
         : require_apply0
      {
         template< typename Parser, typename... States >
         static void apply0( Parser& parser, std::vector< typename Parser::value_t >& captured, Parser::value_t& value, const unsigned /*unused*/, States&&... /*unused*/ )
         {
            Form::build( parser, value, captured );
         }
      };

      template< typename Consumer,
                typename Expression,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      class parser
      {
         static constexpr bool has_consumer_context = requires { typename Consumer::context_t; };

      public:
         using consumer_t = Consumer;
         using value_t = Consumer::value_t;
         using ignored_t = Expression::ignored_t;

         explicit parser( ParseInput& in, States&... states )
            : m_input( in ),
              m_states( states... )
         {}

         [[nodiscard]] std::optional< value_t > parse_expression( const unsigned minimum )
         {
            using forms_t = Expression::forms_t;
            using grammar_context_t = construction_context< parser >;
            using initial_t = forms_t::template initial< grammar_context_t >;
            using trailing_t = forms_t::template trailing< grammar_context_t >;

            std::optional< value_t > left;
            if( !match_form< initial_t >( left ) ) {
               return std::nullopt;
            }
            assert( left );
            while( true ) {
               skip();
               if( !match_form< trailing_t >( *left, minimum ) ) {
                  return left;
               }
            }
         }

         template< typename Rule, typename... SelectedStates >
         [[nodiscard]] bool match_form( SelectedStates&... states )
         {
            std::vector< value_t > captured;
            return std::apply(
               [ & ]( auto&... original ) {
                  return Control< Rule >::template match< apply_mode::enabled, rewind_mode::required, construction_action, Control >( m_input, *this, captured, states..., original... );
               },
               m_states );
         }

         void skip()
         {
            using skip_rule = star< typename Expression::ignored_t >;
            std::apply(
               [ & ]( auto&... states ) {
                  ( void )Control< skip_rule >::template match< apply_mode::disabled, rewind_mode::optional, nothing, Control >( m_input, states... );
               },
               m_states );
         }

         template< typename Binding >
         [[nodiscard]] std::optional< value_t > parse_atom()
         {
            using binding_t = atom_binding_t< Binding >;
            using rule_t = atom_rule_t< Binding >;

            using output_t = atom_output< parser, rule_t >;
            using producer_t = binding_t::template producer_t< rule_t >;
            using selected_action = producer_t::template action< output_t >;
            using action_t = TAO_PEGTL_NAMESPACE::internal::single_rule_action< rule_t, selected_action >;

            std::optional< value_t > result;
            output_t output( *this, result );
            if( !std::apply(
                   [ & ]( auto&... states ) {
                      return Control< rule_t >::template match< apply_mode::enabled, rewind_mode::required, action_t::template type, Control >( m_input, output, states... );
                   },
                   m_states ) ) {
               return std::nullopt;
            }
            assert( result );
            return result;
         }

         template< typename Rule, typename Object >
         [[nodiscard]] value_t atom( Object&& object )
         {
            if constexpr( has_consumer_context ) {
               return Consumer::template atom< Rule >( consumer_context(), std::forward< Object >( object ) );
            }
            else {
               return Consumer::template atom< Rule >( std::forward< Object >( object ) );
            }
         }

         template< typename Form, typename... Args >
         [[nodiscard]] value_t apply( Args&&... args )
         {
            if constexpr( has_consumer_context ) {
               return Consumer::template apply< Form >( consumer_context(), std::forward< Args >( args )... );
            }
            else {
               return Consumer::template apply< Form >( std::forward< Args >( args )... );
            }
         }

      private:
         [[nodiscard]] decltype( auto ) consumer_context()
            requires has_consumer_context
         {
            return std::get< typename Consumer::context_t& >( m_states );
         }

         ParseInput& m_input;
         std::tuple< States&... > m_states;
      };

      template< template< typename > class Consumer >
      struct expression_producer
      {
         template< typename Output >
         struct action
            : maybe_nothing
         {
            template< typename Rule,
                      apply_mode A,
                      rewind_mode M,
                      template< typename... > class Action,
                      template< typename... > class Control,
                      typename ParseInput,
                      typename... States >
            [[nodiscard]] static bool match( ParseInput& in, Output& out, States&&... states )
            {
               static_assert( requires { typename Rule::expression_t; }, "expression_to requires an expression rule" );

               if constexpr( A == apply_mode::enabled ) {
                  using parser_t = parser< Consumer< typename Output::value_t >,
                                           Rule,
                                           Control,
                                           ParseInput,
                                           States... >;

                  parser_t parser( in, states... );
                  if( auto result = parser.parse_expression( 0 ) ) {
                     out.store( std::move( *result ) );
                     return true;
                  }
                  return false;
               }
               else {
                  return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, out, states... );
               }
            }
         };
      };

   }  // namespace internal

   template< typename Ignored, typename... Forms >
   struct expression
   {
      // Recognition uses the caller's actions. Construction instead uses
      // the internal actions above, except for each atom's selected builder.
      using expression_t = expression;
      using ignored_t = Ignored;
      using forms_t = forms< Forms... >;

      using recognition_t = internal::recognize_expression< expression >;
      using rule_t = recognition_t::rule_t;
      using subs_t = recognition_t::subs_t;

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... states )
      {
         return TAO_PEGTL_NAMESPACE::match< internal::recognize_expression< expression >, A, M, Action, Control >( in, states... );
      }
   };

   template< auto S, template< typename > class Consumer = consumer_base >
   using expression_to = build_to< S, internal::expression_producer< Consumer > >;

   template< typename Rule, template< typename > class Consumer = consumer_base >
   struct expression_for
      : build_for< Rule, internal::expression_producer< Consumer > >
   {
      static_assert( requires { typename Rule::expression_t; }, "expression_for requires an expression rule" );
   };

}  // namespace TAO_PEGTL_NAMESPACE::experimental

#endif
