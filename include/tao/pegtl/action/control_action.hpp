// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_ACTION_CONTROL_ACTION_HPP
#define TAO_PEGTL_ACTION_CONTROL_ACTION_HPP

#include <utility>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../match.hpp"
#include "../nothing.hpp"
#include "../rewind_mode.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   struct control_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st )
      {
#if defined( __cpp_exceptions )
         if constexpr( requires { Action< Rule >::unwind( std::declval< const ParseInput& >(), std::declval< States&& >()... ); } ) {
            try {
               return control_action::match_impl< Rule, A, M, Action, Control >( in, st... );
            }
            catch( ... ) {
               Action< Rule >::unwind( std::as_const( in ), st... );
               throw;
            }
         }
         else {
            return control_action::match_impl< Rule, A, M, Action, Control >( in, st... );
         }
#else
         return control_action::match_impl< Rule, A, M, Action, Control >( in, st... );
#endif
      }

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& /*unused*/, States&&... /*unused*/ ) noexcept
      {}

   private:
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match_impl( ParseInput& in, States&&... st )
      {
         Action< Rule >::start( std::as_const( in ), st... );
         if( TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... ) ) {
            Action< Rule >::success( std::as_const( in ), st... );
            return true;
         }
         Action< Rule >::failure( std::as_const( in ), st... );
         return false;
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE

#endif
