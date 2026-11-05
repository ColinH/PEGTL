// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_INPUT_WITH_FUNCS_HPP
#define TAO_PEGTL_INTERNAL_INPUT_WITH_FUNCS_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Input >
   struct input_with_funcs
      : Input
   {
      using Input::Input;

      using data_t = Input::data_t;

      [[nodiscard]] const data_t& peek( const std::size_t offset = 0 ) const noexcept
      {
         return *this->current( offset );
      }

      template< typename T >
      [[nodiscard]] T peek_as( const std::size_t offset = 0 ) const noexcept
         requires( sizeof( data_t ) == sizeof( T ) ) && requires( const data_t& value ) {
            static_cast< T >( value );
         }
      {
         return static_cast< T >( peek( offset ) );
      }

      [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept
         requires requires { peek_as< char >( offset ); }
      {
         return peek_as< char >( offset );
      }

      [[nodiscard]] std::byte peek_byte( const std::size_t offset = 0 ) const noexcept
         requires requires { peek_as< std::byte >( offset ); }
      {
         return peek_as< std::byte >( offset );
      }

      [[nodiscard]] std::int8_t peek_int8( const std::size_t offset = 0 ) const noexcept
         requires requires { peek_as< std::int8_t >( offset ); }
      {
         return peek_as< std::int8_t >( offset );
      }

      [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept
         requires requires { peek_as< std::uint8_t >( offset ); }
      {
         return peek_as< std::uint8_t >( offset );
      }

      [[nodiscard]] std::string string() const
         requires requires( const Input& in )
      {
         static_cast< const char* >( in.current() );
         in.size();
      }
      {
         return std::string( static_cast< const char* >( this->current() ), this->size() );
      }

      [[nodiscard]] std::string_view string_view() const noexcept
         requires requires( const Input& in )
      {
         static_cast< const char* >( in.current() );
         in.size();
      }
      {
         return std::string_view( static_cast< const char* >( this->current() ), this->size() );
      }

      [[nodiscard]] std::span< const data_t > span() const noexcept
         requires requires( const Input& in ) { in.size(); }
      {
         return std::span< const data_t >( this->current(), this->size() );
      }

      [[nodiscard]] std::vector< data_t > vector() const
         requires( std::is_same_v< data_t, std::remove_cv_t< data_t > > && std::is_copy_constructible_v< data_t > ) && requires( const Input& in ) {
            in.size();
         }
      {
         return std::vector< data_t >( this->current(), this->current() + this->size() );
      }

      template< typename Position >
      [[nodiscard]] std::string_view line_view_at( const Position& pos )
         requires requires( Input& in ) {
            static_cast< const char* >( in.begin_of_line( pos ) );
            static_cast< const char* >( in.end_of_line_or_file( pos ) );
         }
      {
         const char* const b = static_cast< const char* >( this->begin_of_line( pos ) );
         const char* const e = static_cast< const char* >( this->end_of_line_or_file( pos ) );
         return { b, static_cast< std::size_t >( e - b ) };
      }

      // TODO: Implement string(), string_view(), line_view_at() with std::u32string, std::u16string_view...?
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
