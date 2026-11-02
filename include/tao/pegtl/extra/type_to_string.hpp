// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_TYPE_TO_STRING_HPP
#define TAO_PEGTL_EXTRA_TYPE_TO_STRING_HPP

#include <cstddef>
#include <string>
#include <string_view>

#include "../config.hpp"

#include "../internal/fixed_string.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace internal
   {
      template< char... Cs >
      struct chars_to_string
      {
         static constexpr char array[] = { Cs..., '$' };

         [[nodiscard]] static std::string string()
         {
            return std::string( array, sizeof...( Cs ) );
         }

         [[nodiscard]] constexpr static std::string_view string_view() noexcept
         {
            return std::string_view( array, sizeof...( Cs ) );
         }
      };

      template< typename >
      struct template_to_string;

      template< template< char... > class T, char... Cs >
      struct template_to_string< T< Cs... > >
         : chars_to_string< Cs... >
      {};

      template< template< std::size_t, char... > class T, std::size_t Prefix, char... Cs >
      struct template_to_string< T< Prefix, Cs... > >
         : chars_to_string< Cs... >
      {};

      template< template< fixed_string > class T, fixed_string Text >
      struct template_to_string< T< Text > >
         : string_to_chars< Text >::template apply< chars_to_string >
      {};

      template< template< std::size_t, fixed_string > class T, std::size_t Prefix, fixed_string Text >
      struct template_to_string< T< Prefix, Text > >
         : string_to_chars< Text >::template apply< chars_to_string >
      {};

   }  // namespace internal

   template< typename T >
   [[nodiscard]] std::string type_to_string()
   {
      return internal::template_to_string< T >::string();
   }

   template< char... Cs >
   [[nodiscard]] std::string type_to_string()
   {
      return internal::chars_to_string< Cs... >::string();
   }

   template< typename T >
   [[nodiscard]] constexpr std::string_view type_to_string_view() noexcept
   {
      return internal::template_to_string< T >::string_view();
   }

   template< char... Cs >
   [[nodiscard]] constexpr std::string_view type_to_string_view() noexcept
   {
      return internal::chars_to_string< Cs... >::string_view();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif
