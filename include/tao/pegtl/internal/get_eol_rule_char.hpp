// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_GET_EOL_RULE_CHAR_HPP
#define TAO_PEGTL_INTERNAL_GET_EOL_RULE_CHAR_HPP

#include <type_traits>

#include "../config.hpp"

#include "one.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   struct get_eol_rule_char
   {
      static constexpr bool value = false;
   };

   template< typename Peek, auto C >
   struct get_eol_rule_char< one< Peek, C > >
   {
      static constexpr auto value = C;
   };

   template< typename Rule >
      requires( !std::is_same_v< Rule, typename Rule::rule_t > )
   struct get_eol_rule_char< Rule >
      : get_eol_rule_char< typename Rule::rule_t >
   {};

   template< typename T >
   inline constexpr auto get_eol_rule_char_v = get_eol_rule_char< T >::value;

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
