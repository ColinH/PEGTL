// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_ACTION_VALUE_TRAITS_HPP
#define TAO_PEGTL_ACTION_VALUE_TRAITS_HPP

#include <cassert>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../config.hpp"
#include "../parse_error.hpp"

#include "internal/integer_utility.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename T, typename = void >
   struct value_traits;

   template<>
   struct value_traits< char >
   {
      template< typename ActionInput >
      [[nodiscard]] static char convert( const ActionInput& in )
      {
         assert( in.size() == 1 );
         return in.peek_char();
      }
   };

   template<>
   struct value_traits< std::string >
   {
      template< typename ActionInput >
      [[nodiscard]] static std::string convert( const ActionInput& in )
      {
         return in.string();
      }
   };

   template<>
   struct value_traits< std::string_view >
   {
      template< typename ActionInput >
      [[nodiscard]] static std::string_view convert( const ActionInput& in )
      {
         return in.string_view();
      }
   };

   template<>
   struct value_traits< std::vector< char > >
   {
      template< typename ActionInput >
      [[nodiscard]] static std::vector< char > convert( const ActionInput& in )
      {
         return in.vector();
      }
   };

   namespace internal
   {
      template< typename T, auto Convert >
      struct convert_floating
      {
         template< typename ActionInput >
         [[nodiscard]] static T convert( const ActionInput& in )
         {
            std::size_t consumed = 0;
            T value;
            try {
               value = Convert( in.string(), &consumed );
            }
            catch( const std::exception& e ) {
               throw_parse_error( e.what(), in );
            }
            if( consumed != in.size() ) {
               throw_parse_error( "trailing garbage after floating point number", in );
            }
            return value;
         }
      };

   }  // namespace internal

   template<>
   struct value_traits< float >
      : internal::convert_floating< float, static_cast< float ( * )( const std::string&, std::size_t* ) >( &std::stof ) >
   {};

   template<>
   struct value_traits< double >
      : internal::convert_floating< double, static_cast< double ( * )( const std::string&, std::size_t* ) >( &std::stod ) >
   {};

   template< typename T >
      requires internal::is_proper_signed_v< T >
   struct value_traits< T >
   {
      template< typename ActionInput >
      [[nodiscard]] static T convert( const ActionInput& in )
      {
         std::size_t consumed = 0;
         using ll_t = long long;  // NOLINT(google-runtime-int))
         ll_t value;  // NOLINT(cppcoreguidelines-init-variables)
         try {
            value = std::stoll( in.string(), &consumed );
         }
         catch( const std::exception& e ) {
            throw_parse_error( e.what(), in );
         }
         if( consumed != in.size() ) {
            throw_parse_error( "trailing garbage after signed integer", in );
         }
         if constexpr( sizeof( T ) < sizeof( ll_t ) ) {
            if( ( value < static_cast< ll_t >( std::numeric_limits< T >::lowest() ) ) || ( value > static_cast< ll_t >( std::numeric_limits< T >::max() ) ) ) {
               throw_parse_error( "signed integer overflow", in );
            }
         }
         return static_cast< T >( value );
      }
   };

   template< typename T >
      requires internal::is_proper_unsigned_v< T >
   struct value_traits< T >
   {
      template< typename ActionInput >
      [[nodiscard]] static T convert( const ActionInput& in )
      {
         std::size_t consumed = 0;
         using ull_t = unsigned long long;  // NOLINT(google-runtime-int)
         ull_t value;  // NOLINT(cppcoreguidelines-init-variables)
         try {
            value = std::stoull( in.string(), &consumed );
         }
         catch( const std::exception& e ) {
            throw_parse_error( e.what(), in );
         }
         if( consumed != in.size() ) {
            throw_parse_error( "trailing garbage after unsigned integer", in );
         }
         if constexpr( sizeof( T ) < sizeof( ull_t ) ) {
            if( value > static_cast< ull_t >( std::numeric_limits< T >::max() ) ) {
               throw_parse_error( "unsigned integer overflow", in );
            }
         }
         return static_cast< T >( value );
      }
   };

   template< typename T >
   struct value_traits< std::optional< T > >
   {
      template< typename ActionInput >
      [[nodiscard]] static std::optional< T > convert( const ActionInput& in )
      {
         return std::optional< T >( std::in_place, value_traits< T >::convert( in ) );
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE

#endif

