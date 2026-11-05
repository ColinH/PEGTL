// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_FUNCTION_HPP
#define TAO_PEGTL_INTERNAL_FUNCTION_HPP

#include <functional>
#include <utility>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../rewind_mode.hpp"
#include "../type_list.hpp"

#include "enable_control.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< auto Function, typename Arg, typename... States >
   concept can_invoke_function = requires( Arg&& arg, States&... st ) {
      std::invoke_r< bool >( Function, std::forward< Arg >( arg ), st... );
   };

   template< auto Function, typename Arg, typename... States >
   [[nodiscard]] bool invoke_function( Arg&& arg, States&... st ) noexcept( noexcept( std::invoke_r< bool >( Function, std::forward< Arg >( arg ), st... ) ) )
      requires can_invoke_function< Function, Arg, States... >
   {
      return std::invoke_r< bool >( Function, std::forward< Arg >( arg ), st... );
   }

   template< auto Function, typename Arg, typename... States >
   [[nodiscard]] bool invoke_function( Arg&& arg, States&... /*unused*/ ) noexcept( noexcept( std::invoke_r< bool >( Function, std::forward< Arg >( arg ) ) ) )
      requires( !can_invoke_function< Function, Arg, States... > && can_invoke_function< Function, Arg > )
   {
      return std::invoke_r< bool >( Function, std::forward< Arg >( arg ) );
   }

   template< typename Peek, auto Function >
   struct function
   {
      using peek_t = Peek;
      using rule_t = function;
      using subs_t = empty_list;

      template< typename ParseInput >
      using peek_result_t = decltype( Peek::peek( std::declval< ParseInput& >() ) );

      template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st ) noexcept( noexcept( Peek::peek( in ) ) && noexcept( static_cast< bool >( std::declval< const peek_result_t< ParseInput >& >() ) ) && noexcept( invoke_function< Function >( std::declval< const peek_result_t< ParseInput >& >().data(), st... ) ) && noexcept( in.template consume< function >( std::declval< const peek_result_t< ParseInput >& >().size() ) ) )
         requires requires { invoke_function< Function >( std::declval< const peek_result_t< ParseInput >& >().data(), st... ); }
      {
         if( const auto t = Peek::peek( in ) ) {
            if( invoke_function< Function >( t.data(), st... ) ) {
               in.template consume< function >( t.size() );
               return true;
            }
         }
         return false;
      }
   };

   template< auto Function >
   struct function< void, Function >
   {
      using rule_t = function;
      using subs_t = empty_list;

      template< apply_mode,
                rewind_mode,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... st ) noexcept( noexcept( invoke_function< Function >( in, st... ) ) )
         requires requires { invoke_function< Function >( in, st... ); }
      {
         return invoke_function< Function >( in, st... );
      }
   };

   template< typename Peek, auto Function >
   inline constexpr bool enable_control< function< Peek, Function > > = false;

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
