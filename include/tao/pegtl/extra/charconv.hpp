// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_CHARCONV_HPP
#define TAO_PEGTL_EXTRA_CHARCONV_HPP

#include <charconv>
#include <cstdint>

#include "../config.hpp"

#include "../debug/analyze_traits.hpp"

#include "internal/charconv.hpp"

#include "overflow_mode.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Integral, std::uint8_t Base, overflow_mode Over >
   struct from_chars_combo
      : internal::from_chars_rule< Integral, Base, Over >,
        internal::from_chars_action< Integral, Base, Over >
   {
      static_assert( Base >= 2 );
      static_assert( Base <= 36 );
   };

   template< typename Integral, std::uint8_t Base = 10 >
   struct from_chars_throws
      : from_chars_combo< Integral, Base, overflow_mode::global_failure >
   {};

   template< typename Integral, std::uint8_t Base = 10 >
   struct from_chars_nothrow
      : from_chars_combo< Integral, Base, overflow_mode::local_failure >
   {};

   template< typename Integral >
   struct from_xchars_throws
      : from_chars_throws< Integral, 16 >
   {};

   template< typename Integral >
   struct from_xchars_nothrow
      : from_chars_nothrow< Integral, 16 >
   {};

#if defined( TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS )
   template< typename Floating, std::chars_format Format, overflow_mode Over >
   struct from_fchars_combo
      : internal::from_chars_rule< Floating, Format, Over >,
        internal::from_chars_action< Floating, Format, Over >
   {
      static_assert( Format == std::chars_format::general || Format == std::chars_format::fixed || Format == std::chars_format::scientific || Format == std::chars_format::hex );
   };

   template< typename Floating, std::chars_format Format = std::chars_format::general >
   struct from_fchars_throws
      : from_fchars_combo< Floating, Format, overflow_mode::global_failure >
   {};

   template< typename Floating, std::chars_format Format = std::chars_format::general >
   struct from_fchars_nothrow
      : from_fchars_combo< Floating, Format, overflow_mode::local_failure >
   {};
#endif

   template< typename Name, typename Number, auto Format, overflow_mode Over >
   struct analyze_traits< Name, internal::from_chars_rule< Number, Format, Over > >
      : analyze_any_traits<>
   {};

}  // namespace TAO_PEGTL_NAMESPACE

#endif
