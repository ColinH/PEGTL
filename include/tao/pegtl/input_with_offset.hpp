// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INPUT_WITH_OFFSET_HPP
#define TAO_PEGTL_INPUT_WITH_OFFSET_HPP

#include <concepts>
#include <utility>

#include "config.hpp"

#if defined( __cpp_exceptions )
#include "parse_error.hpp"
#endif

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Input >
   class input_with_offset
      : public Input
   {
   public:
      using data_t = Input::data_t;
      using error_position_t = Input::error_position_t;
      using offset_position_t = Input::offset_position_t;
      using rewind_position_t = Input::rewind_position_t;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      template< typename... Ts >
         requires std::constructible_from< Input, Ts... >
      explicit input_with_offset( offset_position_t&& s, Ts&&... ts )
         : Input( std::forward< Ts >( ts )... ),
           m_offset( std::move( s ) )
      {}

      template< typename... Ts >
         requires std::constructible_from< Input, Ts... >
      explicit input_with_offset( const offset_position_t& s, Ts&&... ts )
         : Input( std::forward< Ts >( ts )... ),
           m_offset( s )
      {}

      [[nodiscard]] auto current_position() const
      {
         auto pos = Input::current_position();
         pos.apply_offset( m_offset );
         return pos;
      }

      [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const
      {
         auto pos = Input::previous_position( saved );
         pos.apply_offset( m_offset );
         return pos;
      }

      [[nodiscard]] const offset_position_t& direct_offset() const noexcept
      {
         return m_offset;
      }

      void direct_position() const = delete;

   protected:
      offset_position_t m_offset;
   };

}  // namespace TAO_PEGTL_NAMESPACE

#endif
