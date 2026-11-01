// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_STREAM_INPUT_BASE_HPP
#define TAO_PEGTL_STREAM_STREAM_INPUT_BASE_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

#if defined( __cpp_exceptions )
#include <stdexcept>
#else
#include <cstdio>
#include <exception>
#endif
#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Buffer >
   class stream_input_base
      : public Buffer
   {
   public:
      using data_t = Buffer::data_t;

      template< typename... As >
         requires( ( !std::same_as< stream_input_base, std::remove_cvref_t< As > > ) && ... ) && std::constructible_from< Buffer, As... >
      explicit stream_input_base( As&&... as )
         : Buffer( std::forward< As >( as )... ),
           m_current( this->buffer_start() ),
           m_end( this->buffer_start() )
      {}

      [[nodiscard]] bool empty()
      {
         return size( 1 ) == 0;
      }

      [[nodiscard]] std::size_t size( const std::size_t amount )
      {
         require( amount );
         return buffer_used_size();
      }

      [[nodiscard]] std::size_t size( const std::size_t minimum, const std::size_t maximum )
      {
         require( maximum );
         buffer_check_size( minimum );
         return buffer_used_size();
      }

      [[nodiscard]] const data_t* current( const std::size_t offset = 0 ) const noexcept
      {
         assert( m_current + offset <= m_end );
         buffer_assert_consistency();
         return m_current + offset;
      }

      [[nodiscard]] const data_t* end( const std::size_t amount )
      {
         require( amount );
         return m_end;
      }

      [[nodiscard]] const data_t* end( const std::size_t minimum, const std::size_t maximum )
      {
         require( maximum );
         buffer_check_size( minimum );
         return m_end;
      }

      void consume( const std::size_t count ) noexcept
      {
         assert( count <= this->buffer_used_size() );
         buffer_assert_consistency();
         m_current += count;
         buffer_assert_consistency();
      }

      void discard() noexcept
      {
         discard( buffer_used_size() );
      }

      void discard( const std::size_t keep ) noexcept
      {
         buffer_assert_consistency();

         if( keep > 0 ) {
            assert( keep <= buffer_used_size() );
            std::memmove( this->mutable_start(), m_current, keep );
         }
         m_current = this->buffer_start();
         m_end = m_current + keep;

         buffer_assert_consistency();
      }

      void require( const std::size_t amount )
      {
         // assert( amount is not excessive );

         buffer_assert_consistency();

         if( amount <= buffer_used_size() ) {
            return;
         }
         if( ( m_current + amount ) > ( this->buffer_start() + this->buffer_capacity() ) ) {
#if defined( __cpp_exceptions )
            throw std::overflow_error( "require() beyond end of buffer" );
#else
            std::fputs( "overflow error: require() beyond end of buffer\n", stderr );
            std::terminate();
#endif
         }
         m_end += this->m_reader.read( const_cast< data_t* >( m_end ), ( std::min )( buffer_free_after_end(), ( std::max )( amount - buffer_used_size(), this->buffer_chunk_size() ) ) );

         buffer_assert_consistency();
      }

      void buffer_check_size( const std::size_t amount ) const
      {
         buffer_assert_consistency();

         if( buffer_used_size() < amount ) {
#if defined( __cpp_exceptions )
            throw std::overflow_error( "require() beyond end of reader" );
#else
            std::fputs( "overflow error: require() beyond end of reader\n", stderr );
            std::terminate();
#endif
         }
      }

      [[nodiscard]] std::size_t buffer_used_size() const noexcept
      {
         buffer_assert_consistency();
         return m_end - m_current;
      }

      [[nodiscard]] std::size_t buffer_free_before_current() const noexcept
      {
         buffer_assert_consistency();
         return std::size_t( m_current - this->buffer_start() );
      }

      [[nodiscard]] std::size_t buffer_free_after_end() const noexcept
      {
         buffer_assert_consistency();
         return std::size_t( this->buffer_start() + this->buffer_capacity() - m_end );
      }

      void buffer_assert_consistency() const noexcept
      {
         assert( this->buffer_start() <= m_current );
         assert( m_current <= m_end );
         assert( m_end <= this->buffer_start() + this->buffer_capacity() );
      }

   protected:
      const data_t* m_current;
      const data_t* m_end;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
