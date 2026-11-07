// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_ASCII_UTILITY_HPP
#define TAO_PEGTL_INTERNAL_ASCII_UTILITY_HPP

#include <cassert>
#include <cstring>
#include <initializer_list>
#include <type_traits>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   [[nodiscard]] constexpr bool is_seven_value( const T t ) noexcept
   {
      if constexpr( std::is_unsigned_v< T > ) {
         return t <= 127;
      }
      else if constexpr( sizeof( T ) == 1 ) {
         return 0 <= t;
      }
      else {
         return ( 0 <= t ) && ( t <= 127 );
      }
   }

   [[nodiscard]] constexpr bool is_ascii_alpha( const char c ) noexcept
   {
      return ( ( 'a' <= c ) && ( c <= 'z' ) ) || ( ( 'A' <= c ) && ( c <= 'Z' ) );
   }

   template< char C, typename D >
   [[nodiscard]] constexpr bool ascii_char_equal( const D d ) noexcept
   {
      return d == static_cast< D >( C );
   }

   template< char... Cs >
   [[nodiscard]] constexpr bool ascii_string_equal( const void* r ) noexcept
   {
      if constexpr( sizeof...( Cs ) > 0 ) {
         assert( r != nullptr );  // Shut up a clang-analyze false positive.
         const char* t = static_cast< const char* >( r );
         return ( ascii_char_equal< Cs >( *t++ ) && ... );
      }
      else {
         (void)r;
         return true;
      }
   }

   template< char... Cs >
   [[nodiscard]] static std::size_t ascii_astring_equal( const void* r, const std::size_t size ) noexcept
   {
      std::size_t result = 0;

      if constexpr( sizeof...( Cs ) > 0 ) {
         const char* p = static_cast< const char* >( r );
         (void)( ( ( result < size ) && ascii_char_equal< Cs >( p[ result ] ) && ( ++result, true ) ) && ... );  // NOLINT(bugprone-inc-dec-in-conditions)
      }
      else {
         (void)r;
         (void)size;
      }
      return result;
   }

   template< char C, typename D >
   [[nodiscard]] constexpr bool ascii_ichar_equal( const D d ) noexcept
   {
      if constexpr( is_ascii_alpha( C ) ) {
         return ( C | 0x20 ) == ( d | 0x20 );
      }
      else {
         return d == static_cast< D >( C );
      }
   }

   template< char... Cs >
   [[nodiscard]] constexpr bool ascii_istring_equal( const void* r ) noexcept
   {
      if constexpr( sizeof...( Cs ) > 0 ) {
         const char* t = static_cast< const char* >( r );
         return ( ascii_ichar_equal< Cs >( *t++ ) && ... );
      }
      else {
         (void)r;
         return true;
      }
   }

   template< char... Cs >
   [[nodiscard]] static std::size_t ascii_aistring_equal( const void* r, const std::size_t size ) noexcept
   {
      std::size_t result = 0;

      if constexpr( sizeof...( Cs ) > 0 ) {
         const char* p = static_cast< const char* >( r );
         (void)( ( ( result < size ) && ascii_ichar_equal< Cs >( p[ result ] ) && ( ++result, true ) ) && ... );  // NOLINT(bugprone-inc-dec-in-conditions)
      }
      else {
         (void)r;
         (void)size;
      }
      return result;
   }

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
