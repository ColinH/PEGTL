// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_ACTION_INTERNAL_DELIVER_UTILITY_HPP
#define TAO_PEGTL_ACTION_INTERNAL_DELIVER_UTILITY_HPP

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   struct delivery_traits;

   template< typename C, typename... Ts >
   struct delivery_traits_base
   {
      using target_t = C;
      using result_t = std::tuple< std::decay_t< Ts >... >;

      static constexpr std::size_t size = sizeof...( Ts );
   };

   template< typename C, typename T >
   struct delivery_traits_base< C, T >
   {
      using target_t = C;
      using result_t = std::decay_t< T >;

      static constexpr std::size_t size = 1;
   };

   template< typename C, typename T >
      requires( !std::is_function_v< T > )
   struct delivery_traits< T C::* >
      : delivery_traits_base< C, T >
   {};

   template< typename C, typename... Ts >
   struct delivery_traits< void ( * )( C&, Ts... ) >
      : delivery_traits_base< C, Ts... >
   {};

   template< typename C, typename... Ts >
   struct delivery_traits< void ( C::* )( Ts... ) >
      : delivery_traits_base< C, Ts... >
   {};

   template< typename C, typename... Ts >
   struct delivery_traits< void ( * )( C&, Ts... ) noexcept >
      : delivery_traits< void ( * )( C&, Ts... ) >
   {};

   template< typename C, typename... Ts >
   struct delivery_traits< void ( C::* )( Ts... ) noexcept >
      : delivery_traits< void ( C::* )( Ts... ) >
   {};

   template< auto S >
   using delivery_traits_t = delivery_traits< decltype( S ) >;

   template< auto S >
   using delivery_target_t = delivery_traits_t< S >::target_t;

   template< auto S >
   using delivery_result_t = delivery_traits_t< S >::result_t;

   template< auto S, typename Result >
   void deliver( delivery_target_t< S >& target, Result&& result )
   {
      if constexpr( std::is_member_object_pointer_v< decltype( S ) > ) {
         std::invoke( S, target ) = std::forward< Result >( result );
      }
      else if constexpr( delivery_traits_t< S >::size == 1 ) {
         std::invoke( S, target, std::forward< Result >( result ) );
      }
      else {
         std::apply( [ &target ]( auto&&... values ) { std::invoke( S, target, std::forward< decltype( values ) >( values )... ); }, std::forward< Result >( result ) );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
