// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_IS_STREAM_HPP
#define TAO_PEGTL_STREAM_IS_STREAM_HPP

#include <utility>

#include "../config.hpp"
#include "../type_list.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   struct is_stream
   {
      using rule_t = is_stream;
      using subs_t = empty_list;

      template< typename ParseInput >
      [[nodiscard]] static bool match( ParseInput& /*unused*/ ) noexcept
      {
         return requires { std::declval< ParseInput >().direct_reader(); };
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE

#endif
