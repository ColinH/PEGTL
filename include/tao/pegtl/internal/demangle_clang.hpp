// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_DEMANGLE_CLANG_HPP
#define TAO_PEGTL_INTERNAL_DEMANGLE_CLANG_HPP

#include <string_view>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      constexpr std::string_view sv = __PRETTY_FUNCTION__;
      constexpr auto begin = sv.find( '=' );
      static_assert( begin != std::string_view::npos );
      return sv.substr( begin + 2, sv.size() - begin - 3 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif
