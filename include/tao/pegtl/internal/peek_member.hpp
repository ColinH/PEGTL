// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_PEEK_MEMBER_HPP
#define TAO_PEGTL_INTERNAL_PEEK_MEMBER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "../config.hpp"

#include "data_and_size.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename >
   struct peek_member_traits;

   template< typename C, typename T >
   struct peek_member_traits< T C::* >
   {
      using result_t = std::invoke_result_t< T C::*, const C& >;
   };

   template< typename C, typename T >
   struct peek_member_traits< T ( * )( const C& ) >
   {
      using result_t = T;
   };

   template< typename C, typename T >
   struct peek_member_traits< T ( * )( const C& ) noexcept >
      : peek_member_traits< T ( * )( const C& ) >
   {};

   template< auto M >
   struct peek_member
   {
      using result_t = peek_member_traits< decltype( M ) >::result_t;
      using value_t = std::remove_cvref_t< result_t >;
      using data_t = std::remove_cv_t< std::remove_pointer_t< value_t > >;

      static constexpr bool borrowed = std::is_pointer_v< value_t > || std::is_reference_v< result_t >;
      using pair_t = data_and_size< data_t, std::conditional_t< borrowed, void, std::uint8_t > >;

      template< typename ParseInput >
      [[nodiscard]] static pair_t peek( ParseInput& in, const std::size_t offset = 0 ) noexcept( noexcept( in.size( offset + 1 ) ) && noexcept( std::invoke( M, *in.current( offset ) ) ) )
      {
         if( in.size( offset + 1 ) >= ( offset + 1 ) ) {
            if constexpr( std::is_pointer_v< value_t > ) {
               return pair_t( std::invoke( M, *in.current( offset ) ) );
            }
            else if constexpr( borrowed ) {
               auto&& result = std::invoke( M, *in.current( offset ) );
               return pair_t( std::addressof( result ) );
            }
            else {
               return pair_t( std::invoke( M, *in.current( offset ) ), 1 );
            }
         }
         return pair_t();
      }

      template< typename ParseInput >
      [[nodiscard]] static constexpr bool bulk() noexcept
      {
         return true;
      }

      template< typename ParseInput >
      [[nodiscard]] static constexpr std::size_t size() noexcept
      {
         return 1;
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
