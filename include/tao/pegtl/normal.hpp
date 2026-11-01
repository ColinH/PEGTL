// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_NORMAL_HPP
#define TAO_PEGTL_NORMAL_HPP

#include <exception>
#include <string>
#include <type_traits>
#include <utility>

#include "action_input.hpp"
#include "apply_mode.hpp"
#include "config.hpp"
#include "match.hpp"
#include "rewind_mode.hpp"

#include "internal/dummy_guard.hpp"
#include "internal/enable_control.hpp"
#include "internal/has_error_message.hpp"
#include "internal/rewind_guard.hpp"

#if defined( __cpp_exceptions )
#include "demangle.hpp"
#include "parse_error.hpp"
#else
#include <exception>

#include "internal/dependent_false.hpp"
#endif

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct normal
   {
      static constexpr bool enable = internal::enable_control< Rule >;

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      static auto guard( [[maybe_unused]] ParseInput& in, States&&... /*unused*/ ) noexcept
      {
         if constexpr( M == rewind_mode::optional ) {
            return internal::dummy_guard();
         }
         else {
            return internal::rewind_guard( in );
         }
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... /*unused*/ )
      {
#if defined( __cpp_exceptions )
         if constexpr( internal::has_error_message< Rule > ) {
            throw_parse_error( Rule::error_message, in );
         }
         else {
            throw_parse_error( "parse error matching " + std::string( demangle< Rule >() ), in );
         }
#else
         static_assert( internal::dependent_false< Rule >, "Exception support required for normal< Rule >::raise()!" );
         (void)in;
         std::terminate();
#endif
      }

      template< typename Ambient, typename ParseInput, typename... States >
      [[noreturn]] static void raise_nested( const Ambient& am, const ParseInput& /*unused*/, States&&... /*unused*/ )
      {
#if defined( __cpp_exceptions )
         if constexpr( internal::has_error_message< Rule > ) {
            throw_parse_error_with_nested( Rule::error_message, am );
         }
         else {
            throw_parse_error_with_nested( "parse error matching " + std::string( demangle< Rule >() ), am );
         }
#else
         static_assert( internal::dependent_false< Rule >, "Exception support required for normal< Rule >::raise_nested()!" );
         (void)am;
         std::terminate();
#endif
      }

      template< template< typename... > class Action,
                typename RewindPosition,
                typename ParseInput,
                typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( Action< Rule >::apply( std::declval< const action_input< ParseInput >& >(), st... ) ) )
         requires requires { Action< Rule >::apply( std::declval< const action_input< ParseInput >& >(), st... ); }
      {
         const action_input< ParseInput > ai( begin, in );
         return Action< Rule >::apply( ai, st... );
      }

      template< template< typename... > class Action,
                typename RewindPosition,
                typename ParseInput,
                typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( Action< Rule >::template apply< Rule >( std::declval< const action_input< ParseInput >& >(), st... ) ) )
         requires requires { Action< Rule >::template apply< Rule >( std::declval< const action_input< ParseInput >& >(), st... ); }
      {
         const action_input< ParseInput > ai( begin, in );
         return Action< Rule >::template apply< Rule >( ai, st... );
      }

      template< template< typename... > class Action,
                typename ParseInput,
                typename... States >
      static decltype( auto ) apply0( const ParseInput& /*unused*/, States&&... st ) noexcept( noexcept( Action< Rule >::apply0( st... ) ) )
         requires requires { Action< Rule >::apply0( st... ); }
      {
         return Action< Rule >::apply0( st... );
      }

      template< template< typename... > class Action,
                typename ParseInput,
                typename... States >
      static decltype( auto ) apply0( const ParseInput& /*unused*/, States&&... st ) noexcept( noexcept( Action< Rule >::template apply0< Rule >( st... ) ) )
         requires requires { Action< Rule >::template apply0< Rule >( st... ); }
      {
         return Action< Rule >::template apply0< Rule >( st... );
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
         if constexpr( requires { Action< Rule >::template match< Rule, A, M, Action, Control >( std::declval< ParseInput& >(), std::declval< States&& >()... ); } ) {
            return Action< Rule >::template match< Rule, A, M, Action, Control >( in, st... );
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE

#endif
