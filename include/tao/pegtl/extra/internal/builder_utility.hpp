// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_INTERNAL_BUILDER_UTILITY_HPP
#define TAO_PEGTL_EXTRA_INTERNAL_BUILDER_UTILITY_HPP

#include <type_traits>

#include "../../config.hpp"
#include "../../nothing.hpp"

#include "../../action/internal/builder_utility.hpp"
#include "../../action/internal/integer_utility.hpp"

#include "charconv.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename, typename >
   struct is_from_chars
      : std::false_type
   {};

   template< typename Number, auto Format, overflow_mode Over >
   struct is_from_chars< Number, from_chars_rule< void, Format, Over > >
      : std::bool_constant< is_from_chars_type_v< Number, Format > >
   {};

   template< typename Number, auto Format, overflow_mode Over >
   struct is_from_chars< Number, from_chars_rule< Number, Format, Over > >
      : std::true_type
   {};

   template< typename Object, typename Rule >
   inline constexpr bool is_from_chars_v = is_from_chars< Object, Rule >::value;

   template< typename Number, typename Rule >
   inline constexpr bool is_from_chars_or_void_v = is_from_chars_v< Number, typename Rule::rule_t >;

   template< typename Number >
   inline constexpr bool is_from_chars_or_void_v< Number, void > = true;

   template< typename Number >
   struct from_chars_object_action
   {
      template< typename Rule >
      struct type
         : nothing< Rule >
      {
         static_assert( is_from_chars_or_void_v< Number, Rule >, "from_chars builder requires a compatible charconv rule" );
      };
   };

   template< typename Output >
   struct from_chars_producer_action
      : create_action< void, from_chars_object_action< typename Output::value_t >::template type, Output >
   {
      static_assert( is_proper_integer_v< typename Output::value_t > || is_charconv_floating_v< typename Output::value_t > );
   };

   struct from_chars_producer
   {
      template< typename Output >
      using action = from_chars_producer_action< Output >;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
