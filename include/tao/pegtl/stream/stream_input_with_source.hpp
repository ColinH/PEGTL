// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_STREAM_INPUT_WITH_SOURCE_HPP
#define TAO_PEGTL_STREAM_STREAM_INPUT_WITH_SOURCE_HPP

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "../config.hpp"
#include "../count_position.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../pointer_position.hpp"
#include "../position_with_source.hpp"

#include "stream_input.hpp"
#include "stream_input_base.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Buffer, typename InputSource, typename ErrorSource >
   class stream_input_with_source
      : public stream_input_base< Buffer >
   {
   public:
      using data_t = Buffer::data_t;
      using input_source_t = InputSource;
      using error_source_t = ErrorSource;
      using error_position_t = position_with_source< ErrorSource, count_position >;
      using offset_position_t = count_position;
      using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      static_assert( std::is_same_v< InputSource, ErrorSource >, "TODO?" );

      template< typename S, typename... Ts >
         requires( !std::same_as< stream_input_with_source, std::remove_cvref_t< S > > ) && std::constructible_from< error_position_t, S > && std::constructible_from< stream_input_base< Buffer >, Ts... >
      explicit stream_input_with_source( S&& s, Ts&&... ts )
         : stream_input_base< Buffer >( std::forward< Ts >( ts )... ),
           m_position( std::forward< S >( s ) )
      {}

      stream_input_with_source( stream_input_with_source&& ) = delete;
      stream_input_with_source( const stream_input_with_source& ) = delete;

      ~stream_input_with_source() = default;

      void operator=( stream_input_with_source&& ) = delete;
      void operator=( const stream_input_with_source& ) = delete;

      [[nodiscard]] const data_t* previous( const error_position_t saved ) const noexcept
      {
         return this->m_current - m_position.count + saved.count;
      }

      [[nodiscard]] const data_t* previous( const rewind_position_t saved ) const noexcept
      {
         return saved.data;
      }

      template< typename Rule >
      void consume( const std::size_t count ) noexcept
      {
         // assert( count <= buffer_used_size() );
         this->m_current += count;
         m_position.count += count;
      }

      [[nodiscard]] auto rewind_position() const noexcept
      {
         return rewind_position_t( this->m_current );
      }

      void rewind_to_position( const rewind_position_t saved ) noexcept
      {
         m_position.count += ( saved.data - this->m_current );
         this->m_current = saved.data;
      }

      [[nodiscard]] const error_position_t& current_position() const noexcept
      {
         return m_position;
      }

      [[nodiscard]] error_position_t previous_position( const rewind_position_t saved ) const noexcept
      {
         return error_position_t( m_position.source, m_position.count + saved.data - this->m_current );
      }

      [[nodiscard]] std::size_t direct_count() const noexcept
      {
         return m_position.count;
      }

      [[nodiscard]] const error_source_t& direct_source() const noexcept
      {
         return m_position.source;
      }

      [[nodiscard]] const error_position_t& direct_position() const noexcept
      {
         return m_position;
      }

   protected:
      error_position_t m_position;
   };

   template< typename Buffer >
   class stream_input_with_source< Buffer, void, void >
      : public stream_input< Buffer >
   {
   public:
      using stream_input< Buffer >::stream_input;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
