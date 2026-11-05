// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <utility>

#include <tao/pegtl/normal.hpp>
#include <tao/pegtl/nothing.hpp>
#include <tao/pegtl/view_input.hpp>

#include <tao/pegtl/control/internal/apply_typed_state.hpp>
#include <tao/pegtl/control/internal/input_control.hpp>
#include <tao/pegtl/control/internal/remove_first_state.hpp>
#include <tao/pegtl/control/internal/state_control.hpp>
#include <tao/pegtl/control/remove_last_states.hpp>
#include <tao/pegtl/internal/apply0_impl.hpp>
#include <tao/pegtl/internal/apply_impl.hpp>
#include <tao/pegtl/internal/match_impl.hpp>

namespace TAO_PEGTL_NAMESPACE::constraint_tests
{
   struct missing
   {};

   template< typename Result, bool Noexcept = true >
   struct callbacks
   {
      static Result apply( const int&, int& ) noexcept( Noexcept );
      static Result apply0( int& ) noexcept( Noexcept );
   };

   template< typename Action >
   concept can_apply = requires( const int& in, int& state ) {
      { internal::apply_impl< Action >::apply( in, state ) } -> std::same_as< bool >;
   };

   template< typename Action >
   concept can_apply0 = requires( int& state ) {
      { internal::apply0_impl< Action >::apply0( state ) } -> std::same_as< bool >;
   };

   static_assert( can_apply< callbacks< void > > );
   static_assert( can_apply< callbacks< bool > > );
   static_assert( !can_apply< callbacks< int > > );
   static_assert( !can_apply< callbacks< bool& > > );
   static_assert( !can_apply< missing > );
   static_assert( can_apply0< callbacks< void > > );
   static_assert( can_apply0< callbacks< bool > > );
   static_assert( !can_apply0< callbacks< int > > );
   static_assert( !can_apply0< callbacks< bool& > > );
   static_assert( !can_apply0< missing > );

   // Callback arguments remain lvalues even when the adapter receives temporaries.
   static_assert( requires {
      internal::apply_impl< callbacks< void > >::apply( 0, 0 );
      internal::apply0_impl< callbacks< void > >::apply0( 0 );
   } );
   static_assert( noexcept( internal::apply_impl< callbacks< void > >::apply( 0, 0 ) ) );
   static_assert( !noexcept( internal::apply_impl< callbacks< void, false > >::apply( 0, 0 ) ) );
   static_assert( noexcept( internal::apply0_impl< callbacks< bool > >::apply0( 0 ) ) );
   static_assert( !noexcept( internal::apply0_impl< callbacks< bool, false > >::apply0( 0 ) ) );

   struct input
      : view_input<>
   {
      template< typename Rule >
      static constexpr bool enable = false;
   };

   using position = input::rewind_position_t;

   struct disabled_state
   {
      template< typename Rule >
      static constexpr bool enable = false;
   };

   template< typename Rule >
   struct direct_action
   {
      template< typename ActionInput >
      static int& apply( const ActionInput&, int& ) noexcept;

      static int& apply0( int& ) noexcept;
   };

   template< typename Rule >
   struct rule_action
   {
      template< typename Matched, typename ActionInput >
      static int& apply( const ActionInput&, int& ) noexcept;

      template< typename Matched >
      static int& apply0( int& ) noexcept;
   };

   template< typename Rule >
   struct both_actions
      : direct_action< Rule >,
        rule_action< Rule >
   {
      using direct_action< Rule >::apply;
      using direct_action< Rule >::apply0;
      using rule_action< Rule >::apply;
      using rule_action< Rule >::apply0;
   };

   template< typename Control, template< typename... > class Action, typename... States >
   concept has_apply = requires( const position& begin, const input& in, States&... st ) {
      { Control::template apply< Action >( begin, in, st... ) } -> std::same_as< int& >;
   };

   template< typename Control, template< typename... > class Action, typename... States >
   concept has_apply0 = requires( const input& in, States&... st ) {
      { Control::template apply0< Action >( in, st... ) } -> std::same_as< int& >;
   };

   template< typename Control, typename... States >
   constexpr bool verify_adapter()
   {
      static_assert( has_apply< Control, direct_action, States... > );
      static_assert( has_apply< Control, rule_action, States... > );
      static_assert( !has_apply< Control, nothing, States... > );
      static_assert( !has_apply< Control, both_actions, States... > );
      static_assert( has_apply0< Control, direct_action, States... > );
      static_assert( has_apply0< Control, rule_action, States... > );
      static_assert( !has_apply0< Control, nothing, States... > );
      static_assert( !has_apply0< Control, both_actions, States... > );
      return true;
   }

   using base = normal< void >;

   static_assert( verify_adapter< base, int >() );
   static_assert( verify_adapter< internal::remove_first_state< base >, char, int >() );
   static_assert( verify_adapter< remove_last_state_b< base >, int, char >() );
   static_assert( verify_adapter< internal::apply_typed_state< int, base >, char, int >() );
   static_assert( verify_adapter< internal::input_control< normal, void >, int >() );
   static_assert( verify_adapter< internal::state_control< normal, void >, disabled_state, int >() );
   static_assert( !has_apply< base, direct_action, const int > );
   static_assert( !has_apply0< base, direct_action, const int > );

   static_assert( noexcept( base::apply< direct_action >( std::declval< const position& >(), std::declval< const input& >(), std::declval< int& >() ) ) );
   static_assert( noexcept( base::apply0< rule_action >( std::declval< const input& >(), std::declval< int& >() ) ) );

   struct direct_rule
   {
      static int& match( input& )
      {
         static int value = 0;
         return value;
      }
   };

   struct template_rule
   {
      template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control >
      static int& match( input&, int& state )
      {
         return state;
      }
   };

   struct both_rules
      : direct_rule,
        template_rule
   {
      using direct_rule::match;
      using template_rule::match;
   };

   template< typename Rule, typename... States >
   concept has_match = requires( input& in, States&... st ) {
      { internal::match_no_control< Rule, apply_mode::enabled, rewind_mode::optional, nothing, normal >( in, st... ) } -> std::same_as< int& >;
   };

   static_assert( has_match< direct_rule > );
   static_assert( has_match< direct_rule, int > );
   static_assert( has_match< template_rule, int > );
   static_assert( !has_match< template_rule > );
   static_assert( !has_match< both_rules, int > );
   static_assert( !has_match< missing > );

}  // namespace TAO_PEGTL_NAMESPACE::constraint_tests

int main()
{
   return 0;
}
