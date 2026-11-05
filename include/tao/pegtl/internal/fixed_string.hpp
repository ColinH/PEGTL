// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_FIXED_STRING_HPP
#define TAO_PEGTL_INTERNAL_FIXED_STRING_HPP

#include <cstddef>
#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< std::size_t Size >
   struct fixed_string
   {
      char data[ Size ];

      // Literal template arguments need implicit conversion. The loop writes every array element.
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,*-explicit-constructor)
      constexpr fixed_string( const char ( &text )[ Size ] ) noexcept
      {
         for( std::size_t i = 0; i < Size; ++i ) {
            data[ i ] = text[ i ];
         }
      }
   };

   template< fixed_string Text, typename = std::make_index_sequence< sizeof( Text.data ) - 1 > >
   struct string_to_chars;

   template< fixed_string Text, std::size_t... Is >
   struct string_to_chars< Text, std::index_sequence< Is... > >
   {
      template< template< char... > class Rule >
      using apply = Rule< Text.data[ Is ]... >;

      template< template< std::size_t, char... > class Rule, std::size_t Prefix >
      using apply_prefix = Rule< Prefix, Text.data[ Is ]... >;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
