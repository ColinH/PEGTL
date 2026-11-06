// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_BUILDERS_HPP
#define TAO_PEGTL_EXTRA_BUILDERS_HPP

#include <string>

#include "../config.hpp"
#include "../nothing.hpp"

#include "../action/build_for.hpp"
#include "../action/build_to.hpp"
#include "../action/builders.hpp"

#include "charconv.hpp"
#include "unescape.hpp"

#include "internal/builder_utility.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< auto S >
   struct from_chars_to
      : build_to< S, internal::from_chars_producer >
   {
      static_assert( internal::is_proper_integer_v< internal::delivery_result_t< S > > || internal::is_charconv_floating_v< internal::delivery_result_t< S > > );
   };

   template< typename Rule >
   using from_chars_for = build_for< Rule, internal::from_chars_producer >;

   template< auto S, template< typename... > class Unescape = unescape >
   using unescape_to = create_to< S, std::string, Unescape >;

   template< typename Rule, template< typename... > class Unescape = unescape >
   using unescape_for = create_for< Rule, std::string, Unescape >;

}  // namespace TAO_PEGTL_NAMESPACE

#endif
