// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_CONTROL_INTERNAL_INPUT_CONTROL_HPP
#define TAO_PEGTL_CONTROL_INTERNAL_INPUT_CONTROL_HPP

#include "../../config.hpp"

#include "../../internal/has_unwind.hpp"
#include "../../internal/ignore_arguments.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< template< typename... > class Control, typename Rule >
   struct input_control
      : Control< Rule >
   {
      static constexpr bool enable = true;

      template< typename ParseInput, typename... States >
      static void start( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( Control< Rule >::enable ) {
            Control< Rule >::start( in, st... );
         }
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template start< Rule >( in, st... );
         }
         TAO_PEGTL_MSVC_IGNORE( st... );
      }

      template< typename ParseInput, typename... States >
      static void success( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template success< Rule >( in, st... );
         }
         if constexpr( Control< Rule >::enable ) {
            Control< Rule >::success( in, st... );
         }
         TAO_PEGTL_MSVC_IGNORE( st... );
      }

      template< typename ParseInput, typename... States >
      static void failure( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] States&&... st )
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template failure< Rule >( in, st... );
         }
         if constexpr( Control< Rule >::enable ) {
            Control< Rule >::failure( in, st... );
         }
         TAO_PEGTL_MSVC_IGNORE( st... );
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... st )
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template raise< Rule >( in, st... );
         }
         Control< Rule >::raise( in, st... );
      }

      template< typename Ambient, typename ParseInput, typename... States >
      [[noreturn]] static void raise_nested( const Ambient& am, const ParseInput& in, States&&... st )
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template raise_nested< Rule >( am, in, st... );
         }
         Control< Rule >::raise_nested( am, in, st... );
      }

      template< typename ParseInput, typename... States >
      static void unwind( [[maybe_unused]] const ParseInput& in, [[maybe_unused]] States&&... st )
         requires( ParseInput::template enable< Rule > || ( Control< Rule >::enable && internal::has_unwind< Control< Rule >, void, const ParseInput&, States... > ))
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template unwind< Rule >( in, st... );
         }
         if constexpr( Control< Rule >::enable && internal::has_unwind< Control< Rule >, void, const ParseInput&, States... > ) {
            Control< Rule >::unwind( in, st... );
         }
         TAO_PEGTL_MSVC_IGNORE( st... );
      }

      template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, States&&... st )
         requires requires { Control< Rule >::template apply< Action >( begin, in, st... ); }
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template apply< Rule >( in, st... );
         }
         return Control< Rule >::template apply< Action >( begin, in, st... );
      }

      template< template< typename... > class Action, typename ParseInput, typename... States >
      static decltype( auto ) apply0( const ParseInput& in, States&&... st )
         requires requires { Control< Rule >::template apply0< Action >( in, st... ); }
      {
         if constexpr( ParseInput::template enable< Rule > ) {
            const_cast< ParseInput& >( in ).template apply0< Rule >( in, st... );
         }
         return Control< Rule >::template apply0< Action >( in, st... );
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
