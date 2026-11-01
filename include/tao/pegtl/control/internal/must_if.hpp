// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_CONTROL_INTERNAL_MUST_IF_HPP
#define TAO_PEGTL_CONTROL_INTERNAL_MUST_IF_HPP

#if !defined( __cpp_exceptions )
#error "Exception support required for tao/pegtl/control/internal/must_if.hpp"
#else

#include "../../config.hpp"
#include "../../parse_error.hpp"

#include "../../internal/has_error_message.hpp"
#include "../../internal/ignore_arguments.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Errors, typename Rule >
   inline constexpr bool raise_on_failure = ( Errors::template message< Rule > != nullptr );

   template< typename Errors, typename Rule >
      requires requires { Errors::template raise_on_failure< Rule >; }
   inline constexpr bool raise_on_failure< Errors, Rule > = Errors::template raise_on_failure< Rule >;

   template< typename Errors, bool RequireMessage, template< typename... > class Control, typename Rule >
   struct must_if
      : Control< Rule >
   {
      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& in, States&&... st ) noexcept( noexcept( Control< Rule >::failure( in, st... ) ) && !internal::raise_on_failure< Errors, Rule > )
      {
         if constexpr( internal::raise_on_failure< Errors, Rule > ) {
            raise( in, st... );
         }
         else {
            Control< Rule >::failure( in, st... );
         }
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( Errors::template message< Rule > != nullptr ) {
            constexpr const char* p = Errors::template message< Rule >;
            TAO_PEGTL_MSVC_IGNORE( st... );
            throw_parse_error( p, in );
         }
         else {
            if constexpr( !internal::has_error_message< Rule > ) {
               static_assert( !RequireMessage, "Explicit error message required for rule!" );
            }
            Control< Rule >::raise( in, st... );
         }
      }

      template< typename Ambient, typename ParseInput, typename... States >
      [[noreturn]] static void raise_nested( const Ambient& am, [[maybe_unused]] const ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( Errors::template message< Rule > != nullptr ) {
            constexpr const char* p = Errors::template message< Rule >;
            TAO_PEGTL_MSVC_IGNORE( in, st... );
            throw_parse_error_with_nested( p, am );
         }
         else {
            if constexpr( !internal::has_error_message< Rule > ) {
               static_assert( !RequireMessage, "Explicit error message required for rule!" );
            }
            Control< Rule >::raise_nested( am, in, st... );
         }
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#endif
