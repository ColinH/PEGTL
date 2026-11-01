// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_TEXT_INPUT_HPP
#define TAO_PEGTL_INTERNAL_TEXT_INPUT_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../normal.hpp"
#include "../nothing.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../pointer_position.hpp"
#include "../rewind_mode.hpp"
#include "../text_position.hpp"

#include "at.hpp"
#include "eof.hpp"
#include "sor.hpp"
#include "text_consume.hpp"
#include "until.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Eol, typename Input >
   class text_input
      : public Input
   {
   public:
      using data_t = Input::data_t;
      using error_position_t = text_position;
      using offset_position_t = text_position;
      using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      using eol_rule = Eol;

      using Input::Input;

      static_assert( !std::is_same_v< Eol, void > );

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
         Input::rewind_to_position( pointer_position< data_t >( previous( saved ) ) );
         m_position = saved;
      }

      [[nodiscard]] const auto& current_position() const noexcept
      {
         return m_position;
      }

      [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const noexcept
      {
         return saved;
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
         using grammar = until< at< sor< eof, eol_rule > > >;
         const data_t* p = previous( pos );  // TODO: Start earlier?
         const std::size_t s = ( std::min )( max, std::size_t( this->end() - p ) );
         scan_input< data_t > in( p, s );
         (void)normal< grammar >::template match< apply_mode::disabled, rewind_mode::optional, nothing, normal >( in );
         return in.current();
      }

   protected:
      error_position_t m_position;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
