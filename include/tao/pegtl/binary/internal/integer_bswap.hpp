// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_BINARY_INTERNAL_INTEGER_BSWAP_HPP
#define TAO_PEGTL_BINARY_INTERNAL_INTEGER_BSWAP_HPP

#include <bit>
#include <concepts>
#include <type_traits>
#include <utility>

#include "../../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< std::integral T >
   [[nodiscard]] constexpr T integer_bswap( const T n ) noexcept
   {
      if constexpr( sizeof( T ) == 1 ) {
         return n;
      }
      else {
         return std::byteswap( n );
      }
   }

   template< typename T >
      requires std::is_enum_v< T >
   [[nodiscard]] constexpr T integer_bswap( const T n ) noexcept
   {
      return static_cast< T >( integer_bswap( std::to_underlying( n ) ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
