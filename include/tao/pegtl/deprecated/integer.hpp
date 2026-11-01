// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_DEPRECATED_INTEGER_HPP
#define TAO_PEGTL_DEPRECATED_INTEGER_HPP

#if !defined( __cpp_exceptions )
#error "Exception support required for tao/pegtl/deprecated/integer.hpp"
#else

#include <limits>
#include <type_traits>

#include "../ascii.hpp"
#include "../config.hpp"
#include "../parse.hpp"
#include "../parse_error.hpp"
#include "../rules.hpp"

#include "../debug/analyze_traits.hpp"

#include "internal/integer.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   struct unsigned_rule_old
      : plus< digit >
   {
      // Pre-3.0 version of this rule.
   };

   using integer_digits_one = sor< one< '0' >, plus< digit > >;                            // Don't consume after first zero.
   using integer_digits_two = if_then_else< one< '0' >, not_at< digit >, plus< digit > >;  // Don't match when superfluous leading zero(s) are present.

   struct unsigned_rule_new
      : integer_digits_two
   {
      // New version that does not allow leading zeros.
   };

   struct signed_rule_old
      : seq< opt< one< '-', '+' > >, plus< digit > >
   {
      // Pre-3.0 version of this rule.
   };

   struct signed_rule_new
      : seq< opt< one< '-', '+' > >, integer_digits_two >
   {
      // New version that does not allow leading zeros.
   };

   struct signed_rule_bis
      : seq< opt< one< '-' > >, integer_digits_two >
   {
      // Only allow minus as sign like std::from_chars.
   };

   struct signed_rule_ter
      : seq< one< '-', '+' >, integer_digits_two >
   {
      // Require sign, for languages where 42 is unsigned and +42 is signed.
   };

   // There are many more possibilities, for example "allow a sign only for non-zero integers".

   struct unsigned_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits.

      template< typename ActionInput, typename Unsigned >
      static void apply( const ActionInput& in, Unsigned& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_unsigned( st, in.string_view() ) ) {
            throw_parse_error( "unsigned integer overflow", in );
         }
      }
   };

   struct unsigned_rule
   {
      using rule_t = unsigned_rule;
      using subs_t = empty_list;

      template< typename ParseInput, typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) )
      {
         return internal::match_unsigned( in );  // Does not check for any overflow.
      }
   };

   struct unsigned_rule_with_action
   {
      using rule_t = unsigned_rule_with_action;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) )
         requires( A == apply_mode::disabled )
      {
         return internal::match_unsigned( in );  // Does not check for any overflow.
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename Unsigned >
      [[nodiscard]] static bool match( ParseInput& in, Unsigned& st )
         requires( ( A == apply_mode::enabled ) && std::is_unsigned_v< Unsigned > )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         return internal::match_and_convert_unsigned_with_maximum_throws( in, st );  // Throws on overflow.
      }
   };

   template< typename Unsigned, Unsigned Maximum >
   struct maximum_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits.

      static_assert( std::is_unsigned_v< Unsigned > );

      template< typename ActionInput, typename Unsigned2 >
      static void apply( const ActionInput& in, Unsigned2& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_unsigned< Unsigned, Maximum >( st, in.string_view() ) ) {
            throw_parse_error( "unsigned integer overflow", in );
         }
      }
   };

   template< typename Unsigned, Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
   struct maximum_rule
   {
      using rule_t = maximum_rule;
      using subs_t = empty_list;

      static_assert( std::is_unsigned_v< Unsigned > );

      template< typename ParseInput, typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ )
      {
         Unsigned st = 0;
         return internal::match_and_convert_unsigned_with_maximum_nothrow< ParseInput, Unsigned, Maximum >( in, st );
      }
   };

   template< typename Unsigned, Unsigned Maximum = ( std::numeric_limits< Unsigned >::max )() >
   struct maximum_rule_with_action
   {
      using rule_t = maximum_rule_with_action;
      using subs_t = empty_list;

      static_assert( std::is_unsigned_v< Unsigned > );

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ )
         requires( A == apply_mode::disabled )
      {
         Unsigned st = 0;
         return internal::match_and_convert_unsigned_with_maximum_throws< ParseInput, Unsigned, Maximum >( in, st );
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename Unsigned2 >
      [[nodiscard]] static bool match( ParseInput& in, Unsigned2& st )
         requires( ( A == apply_mode::enabled ) && std::is_same_v< Unsigned, Unsigned2 > )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         return internal::match_and_convert_unsigned_with_maximum_throws< ParseInput, Unsigned, Maximum >( in, st );
      }
   };

   struct signed_action
   {
      // Assumes that 'in' contains a non-empty sequence of ASCII digits,
      // with optional leading sign; with sign, in.size() must be >= 2.

      template< typename ActionInput, typename Signed >
      static void apply( const ActionInput& in, Signed& st )
      {
         // This function "only" offers basic exception safety.
         st = 0;
         if( !internal::convert_signed( st, in.string_view() ) ) {
            throw_parse_error( "signed integer overflow", in );
         }
      }
   };

   struct signed_rule
   {
      using rule_t = signed_rule;
      using subs_t = empty_list;

      template< typename ParseInput, typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) )
      {
         return parse< signed_rule_new >( in );  // Does not check for any overflow.
      }
   };

   namespace internal
   {
      template< typename Rule >
      struct signed_action_action
         : nothing< Rule >
      {};

      template<>
      struct signed_action_action< signed_rule_new >
         : signed_action
      {};

   }  // namespace internal

   struct signed_rule_with_action
   {
      using rule_t = signed_rule_with_action;
      using subs_t = empty_list;

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, States&&... /*unused*/ ) noexcept( noexcept( in.empty() ) )
         requires( A == apply_mode::disabled )
      {
         return parse< signed_rule_new >( in );  // Does not check for any overflow.
      }

      template< apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename Signed >
      [[nodiscard]] static bool match( ParseInput& in, Signed& st )
         requires( ( A == apply_mode::enabled ) && std::is_signed_v< Signed > )
      {
         return parse< signed_rule_new, internal::signed_action_action >( in, st );  // Throws on overflow.
      }
   };

   template< typename Name >
   struct analyze_traits< Name, unsigned_rule >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, unsigned_rule_with_action >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Integer, Integer Maximum >
   struct analyze_traits< Name, maximum_rule< Integer, Maximum > >
      : analyze_any_traits<>
   {};

   template< typename Name, typename Integer, Integer Maximum >
   struct analyze_traits< Name, maximum_rule_with_action< Integer, Maximum > >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, signed_rule >
      : analyze_any_traits<>
   {};

   template< typename Name >
   struct analyze_traits< Name, signed_rule_with_action >
      : analyze_any_traits<>
   {};

}  // namespace TAO_PEGTL_NAMESPACE

#endif
#endif
