// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_DEMANGLE_GCC_HPP
#define TAO_PEGTL_INTERNAL_DEMANGLE_GCC_HPP

#if ( __GNUC__ == 16 ) && ( __GNUC_MINOR__ < 2 )

// GCC 16.1 can truncate __PRETTY_FUNCTION__.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91155

#include "demangle_rtti.hpp"

#else

#include <string_view>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace special
   {
      template< typename T >
      [[nodiscard]] constexpr std::string_view demangle() noexcept
      {
         constexpr std::string_view sv = __PRETTY_FUNCTION__;
         constexpr auto begin = sv.find( '=' );
         static_assert( begin != std::string_view::npos );
         constexpr auto tmp = sv.substr( begin + 2 );
         constexpr auto end = tmp.rfind( ';' );
         static_assert( end != std::string_view::npos );
         return tmp.substr( 0, end );
      }

   }  // namespace special

   template< typename T >
   [[nodiscard]] constexpr std::string_view demangle() noexcept
   {
      return special::demangle< T >();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif

#endif
