// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_TEXT_INPUT_WITH_SOURCE_HPP
#define TAO_PEGTL_INTERNAL_TEXT_INPUT_WITH_SOURCE_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../normal.hpp"
#include "../nothing.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../position_with_source.hpp"
#include "../rewind_mode.hpp"
#include "../text_position.hpp"

#include "at.hpp"
#include "eof.hpp"
#include "sor.hpp"
#include "text_consume.hpp"
#include "text_input.hpp"
#include "until.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Eol, typename InputSource, typename ErrorSource, typename Input >
   class text_input_with_source
      : public Input
   {
   public:
      using data_t = Input::data_t;
      using input_source_t = InputSource;
      using error_source_t = ErrorSource;
      using error_position_t = position_with_source< ErrorSource, text_position >;
      using offset_position_t = text_position;
      using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      using eol_rule = Eol;

      static_assert( !std::is_same_v< Eol, void > );
      static_assert( std::is_same_v< InputSource, ErrorSource >, "TODO?" );

      template< typename S, typename... Ts >
         requires( !std::same_as< text_input_with_source, std::remove_cvref_t< S > > ) && std::constructible_from< error_position_t, S > && std::constructible_from< Input, Ts... >
      explicit text_input_with_source( S&& s, Ts&&... ts )
         : Input( std::forward< Ts >( ts )... ),
           m_position( std::forward< S >( s ) )
      {}

      [[nodiscard]] const data_t* start() const noexcept
      {
         return this->m_current - m_position.count;
      }

      void restart() noexcept
      {
         this->m_current -= m_position.count;
         m_position.count = 0;
         m_position.line = 1;
         m_position.column = 1;
      };

      [[nodiscard]] const data_t* previous( const rewind_position_t& saved ) const noexcept
      {
         return this->current() - m_position.count + saved.count;
      }

      template< typename Rule >
      void consume( const std::size_t count )
      {
         scan_input< data_t > in( this->current(), count );
         text_consume< Eol, Rule >( m_position, in );
         this->m_current += count;
      }

      [[nodiscard]] auto rewind_position() const noexcept
      {
         return rewind_position_t( m_position );
      }

      void rewind_to_position( const rewind_position_t& saved ) noexcept
      {
         this->m_current = previous( saved );
         m_position.base() = saved;
      }

      [[nodiscard]] const auto& current_position() const noexcept
      {
         return m_position;
      }

      [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const
      {
         return error_position_t( m_position.source, saved );
      }

      [[nodiscard]] std::size_t direct_count() const noexcept
      {
         return m_position.count;
      }

      [[nodiscard]] std::size_t direct_line() const noexcept
      {
         return m_position.line;
      }

      [[nodiscard]] std::size_t direct_column() const noexcept
      {
         return m_position.column;
      }

      [[nodiscard]] const InputSource& direct_source() const noexcept
      {
         return m_position.source;
      }

      [[nodiscard]] const auto& direct_position() const noexcept
      {
         return m_position;
      }

      [[nodiscard]] const data_t* begin_of_line( const error_position_t& pos, const std::size_t max = 135 ) const noexcept
      {
         // assert( pos.column > 0 );
         return previous( pos ) - ( std::min )( max, pos.column - 1 );
      }

      [[nodiscard]] const data_t* end_of_line_or_file( const error_position_t& pos, const std::size_t max = 135 ) const
         requires( sizeof( data_t ) == 1 )
      {
         // Generalising beyond 1 requires the until to use eol_lazy_peek like in lazy_input.
         using eolof_grammar = until< at< sor< eof, eol_rule > > >;
         const data_t* p = previous( pos );  // TODO: Start earlier?
         const std::size_t s = ( std::min )( max, std::size_t( this->end() - p ) );
         scan_input< data_t > in( p, s );
         (void)normal< eolof_grammar >::template match< apply_mode::disabled, rewind_mode::optional, nothing, normal >( in );
         return in.current();
      }

   protected:
      error_position_t m_position;
   };

   template< typename Eol, typename Input >
   class text_input_with_source< Eol, void, void, Input >
      : public text_input< Eol, Input >
   {
   public:
      using text_input< Eol, Input >::text_input;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
