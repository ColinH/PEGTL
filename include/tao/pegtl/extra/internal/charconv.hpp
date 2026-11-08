// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_INTERNAL_CHARCONV_HPP
#define TAO_PEGTL_EXTRA_INTERNAL_CHARCONV_HPP

#if !defined( __cpp_exceptions )
#error "Exception support required for tao/pegtl/extra/internal/charconv.hpp"
#else

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <type_traits>

#include "../../apply_mode.hpp"
#include "../../config.hpp"
#include "../../parse_error.hpp"
#include "../../rewind_mode.hpp"
#include "../../type_list.hpp"

#include "../../debug/analyze_traits.hpp"

#include "../../internal/enable_control.hpp"

#include "../overflow_mode.hpp"

#include "charconv_features.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
#if defined( TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS )
   template< typename T >
   inline constexpr bool is_charconv_floating_v = std::is_same_v< T, float > || std::is_same_v< T, double >;
#else
   template< typename T >
   inline constexpr bool is_charconv_floating_v = false;
#endif

   template< typename Number, auto Format >
   inline constexpr bool is_from_chars_type_v = ( std::is_same_v< decltype( Format ), std::uint8_t > && std::is_integral_v< Number > )
                                                || ( std::is_same_v< decltype( Format ), std::chars_format > && is_charconv_floating_v< Number > );

   template< overflow_mode O, typename Input, typename Number, typename Format >
   [[nodiscard]] const char* from_chars_end( const Input& in, const char* const begin, const char* const end, Number& value, const Format format )
   {
      const auto result = std::from_chars( begin, end, value, format );
      if constexpr( O == overflow_mode::global_failure ) {
         if( result.ec == std::errc::result_out_of_range ) {
            throw_parse_error( "charconv result out of range", in );
         }
      }
      return result.ec == std::errc{} ? result.ptr : nullptr;
   }

   template< typename Rule, overflow_mode Over, auto Format, typename ParseInput, typename Number >
   [[nodiscard]] bool from_chars_match( ParseInput& in, Number& out )
   {
      const std::size_t size = [ & ]() {
         if constexpr( std::is_same_v< decltype( Format ), std::chars_format > ) {
            return in.size();
         }
         else {
            const std::size_t available = in.size( 3 + ( sizeof( Number ) * 8 ) );
            return in.size( available );
         }
      }();
      const char* const begin = in.current();
      Number value;
      if( const auto* end = from_chars_end< Over >( in, begin, begin + size, value, Format ) ) {
         in.template consume< Rule >( end - begin );
         out = value;
         return true;
      }
      return false;
   }

   template< typename Number, auto Format, overflow_mode Over >
   struct from_chars_rule
   {
      using rule_t = from_chars_rule;
      using subs_t = empty_list;

      static_assert( is_from_chars_type_v< Number, Format > );

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ )
      {
         Number out;
         return from_chars_match< from_chars_rule, Over, Format >( in, out );
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, Number& out, States&&... /*unused*/ )
         requires( A == apply_mode::enabled )
      {
         return from_chars_match< from_chars_rule, Over, Format >( in, out );
      }
   };

   template< auto Format, overflow_mode Over >
   struct from_chars_rule< void, Format, Over >
   {
      using rule_t = from_chars_rule;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename Number,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, Number& out, States&&... /*unused*/ )
         requires is_from_chars_type_v< Number, Format >
      {
         return from_chars_match< from_chars_rule, Over, Format >( in, out );
      }
   };

   template< typename Number, auto Format, overflow_mode Over >
   inline constexpr bool enable_control< from_chars_rule< Number, Format, Over > > = false;

   template< overflow_mode Over, auto Format, typename ActionInput, typename Number >
   [[nodiscard]] bool from_chars_apply( const ActionInput& in, Number& out )
   {
      const char* const begin = in.current();
      const char* const end = begin + in.size();
      Number value;
      if( const auto* parsed = from_chars_end< Over >( in, begin, end, value, Format ); parsed && parsed == end ) {
         out = value;
         return true;
      }
      return false;
   }

   template< typename Number, auto Format, overflow_mode Over >
   struct from_chars_action
   {
      static_assert( is_from_chars_type_v< Number, Format > );

      template< typename ActionInput, typename... States >
      [[nodiscard]] static bool apply( const ActionInput& in, Number& out, States&&... /*unused*/ )
      {
         return from_chars_apply< Over, Format >( in, out );
      }
   };

   template< auto Format, overflow_mode Over >
   struct from_chars_action< void, Format, Over >
   {
      template< typename ActionInput, typename Number, typename... States >
      [[nodiscard]] static bool apply( const ActionInput& in, Number& out, States&&... /*unused*/ )
         requires is_from_chars_type_v< Number, Format >
      {
         return from_chars_apply< Over, Format >( in, out );
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
#endif
