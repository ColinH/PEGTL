// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_BOF_HPP
#define TAO_PEGTL_INTERNAL_BOF_HPP

#include <utility>

#include "../config.hpp"
#include "../type_list.hpp"

#include "enable_control.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   struct bof
   {
      using rule_t = bof;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& in ) noexcept
      {
         if constexpr( requires { std::declval< ParseInput >().start(); } ) {
            return in.current() == in.start();
         }
         else {
            return in.direct_position().count == 0;
         }
      }
   };

   template<>
   inline constexpr bool enable_control< bof > = false;

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
