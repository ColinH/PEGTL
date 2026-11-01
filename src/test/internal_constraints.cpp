// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>
#include <utility>

#include <tao/pegtl/normal.hpp>
#include <tao/pegtl/nothing.hpp>
#include <tao/pegtl/view_input.hpp>

#include <tao/pegtl/control/internal/apply_typed_state.hpp>
#include <tao/pegtl/control/internal/input_control.hpp>
#include <tao/pegtl/control/internal/remove_first_state.hpp>
#include <tao/pegtl/control/internal/state_control.hpp>
#include <tao/pegtl/control/remove_last_states.hpp>
#include <tao/pegtl/extra/parse_tree.hpp>
#include <tao/pegtl/internal/match_impl.hpp>

#if defined( __cpp_exceptions )
#include <tao/pegtl/action/internal/builder_utility.hpp>
#include <tao/pegtl/control/internal/must_if.hpp>
#endif

namespace TAO_PEGTL_NAMESPACE::constraint_tests
{
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
      static int& match( input& );
   };

   struct template_rule
   {
      template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control >
      static int& match( input&, int& );
   };

   struct both_rules
      : direct_rule,
        template_rule
   {
      using direct_rule::match;
      using template_rule::match;
   };

   struct missing
   {};

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

   struct transform_with_input
   {
      static int& transform( const input&, std::unique_ptr< int >&, int& ) noexcept;
   };

   struct transform_without_input
   {
      static int& transform( std::unique_ptr< int >&, int& ) noexcept;
   };

   struct both_transforms
      : transform_with_input,
        transform_without_input
   {
      using transform_with_input::transform;
      using transform_without_input::transform;
   };

   template< typename Selector >
   concept has_transform = requires( const input& in, std::unique_ptr< int >& node, int& state ) {
      { parse_tree::internal::transform< Selector >( in, node, state ) } -> std::same_as< void >;
   };

   // Transform results are discarded; absence selects the no-op fallback.
   static_assert( has_transform< transform_with_input > );
   static_assert( has_transform< transform_without_input > );
   static_assert( has_transform< missing > );
   static_assert( !has_transform< both_transforms > );

#if defined( __cpp_exceptions )
   struct constructed
   {
      explicit constructed( int );
   };

   template< typename T >
   struct constructor_traits
   {};

   template< typename T >
   struct factory_traits
   {
      static const T& make( int&& );
   };

   // The factory retains priority over the constructor and its exact result type.
   static_assert( std::same_as< decltype( internal::make_multi_impl< factory_traits, constructed >( 0, 1 ) ), const constructed& > );
   static_assert( std::same_as< decltype( internal::make_multi_impl< constructor_traits, constructed >( 0, 1 ) ), constructed > );

   struct no_message
   {
      template< typename Rule >
      static constexpr const char* message = nullptr;
   };

   struct with_message
   {
      template< typename Rule >
      static constexpr const char* message = "message";
   };

   struct override_false
      : with_message
   {
      template< typename Rule >
      static constexpr bool raise_on_failure = false;
   };

   struct override_true
   {
      template< typename Rule >
      static constexpr bool raise_on_failure = true;
   };

   static_assert( !internal::raise_on_failure< no_message, void > );
   static_assert( internal::raise_on_failure< with_message, void > );
   static_assert( !internal::raise_on_failure< override_false, void > );
   static_assert( internal::raise_on_failure< override_true, void > );
#endif
}  // namespace TAO_PEGTL_NAMESPACE::constraint_tests

int main()
{
   return 0;
}
