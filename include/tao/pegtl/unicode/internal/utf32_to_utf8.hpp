// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_UNICODE_INTERNAL_UTF32_TO_UTF8_HPP
#define TAO_PEGTL_UNICODE_INTERNAL_UTF32_TO_UTF8_HPP

#include "../../config.hpp"

#include "../../internal/ascii_string.hpp"

#include "utf8_details.hpp"
#include "value_list_concat.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< template< char... > class String, char32_t C >
   struct utf32_to_utf8_char;

   template< template< char... > class String, char32_t C >
      requires( has_utf8_length_1( C ) )
   struct utf32_to_utf8_char< String, C >
   {
      using type = String< static_cast< char >( C ) >;
   };

   template< template< char... > class String, char32_t C >
      requires( has_utf8_length_2( C ) )
   struct utf32_to_utf8_char< String, C >
   {
      using type = String< utf8_char_1_of_2( C ),
                           utf8_char_2_of_2( C ) >;
   };

   template< template< char... > class String, char32_t C >
      requires( has_utf8_length_3( C ) )
   struct utf32_to_utf8_char< String, C >
   {
      using type = String< utf8_char_1_of_3( C ),
                           utf8_char_2_of_3( C ),
                           utf8_char_3_of_3( C ) >;
   };
   template< template< char... > class String, char32_t C >
      requires( has_utf8_length_4( C ) )
   struct utf32_to_utf8_char< String, C >
   {
      using type = String< utf8_char_1_of_4( C ),
                           utf8_char_2_of_4( C ),
                           utf8_char_3_of_4( C ),
                           utf8_char_4_of_4( C ) >;
   };

   template< template< char... > class String, char32_t... Cs >
   struct utf32_to_utf8_list
   {
      using type = value_list_concat_t< typename utf32_to_utf8_char< String, Cs >::type... >;
   };

   template< template< char... > class String, char32_t C >
   struct utf32_to_utf8_list< String, C >
   {
      using type = utf32_to_utf8_char< String, C >::type;
   };

   template< template< char... > class String >
   struct utf32_to_utf8_list< String >
   {
      using type = String<>;
   };

   template< char32_t... Cs >
   using utf32_to_utf8_t = utf32_to_utf8_list< ascii_string, Cs... >::type;

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
