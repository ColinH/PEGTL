// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_INPUT_WITH_START_HPP
#define TAO_PEGTL_INTERNAL_INPUT_WITH_START_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#include "../config.hpp"
#include "../count_position.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../pointer_position.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Input >
   class input_with_start
      : public Input
   {
   public:
      using data_t = Input::data_t;
      using error_position_t = count_position;
      using offset_position_t = count_position;
      using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      template< typename... As >
         requires( ( !std::same_as< input_with_start, std::remove_cvref_t< As > > ) && ... ) && std::constructible_from< Input, As... >
      explicit input_with_start( As&&... as ) noexcept
         : Input( std::forward< As >( as )... ),
           m_start( this->current() )
      {}

      [[nodiscard]] const data_t* start() const noexcept
      {
         return m_start;
      }

      [[nodiscard]] const data_t* previous( const error_position_t saved ) const noexcept
      {
         return m_start + saved.count;
      }

      [[nodiscard]] const data_t* previous( const rewind_position_t saved ) const noexcept
      {
         return saved.data;
      }

      void restart() noexcept
      {
         this->m_current = m_start;
      }

      [[nodiscard]] auto current_position() const noexcept
      {
         return previous_position( this->rewind_position() );
      }

      [[nodiscard]] auto previous_position( const error_position_t saved ) const noexcept
      {
         return saved;
      }

      [[nodiscard]] auto previous_position( const rewind_position_t saved ) const noexcept
      {
         return count_position( saved.data - m_start );
      }

   protected:
      const data_t* m_start;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
