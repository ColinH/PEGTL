// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_INPUT_WITH_SOURCE_HPP
#define TAO_PEGTL_INTERNAL_INPUT_WITH_SOURCE_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#include "../config.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../position_with_source.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename InputSource, typename ErrorSource, typename Input >
   class input_with_source
      : public Input
   {
   public:
      using data_t = Input::data_t;
      using input_source_t = InputSource;
      using error_source_t = ErrorSource;
      using error_position_t = position_with_source< ErrorSource, typename Input::error_position_t >;
      using offset_position_t = Input::offset_position_t;
      using rewind_position_t = Input::rewind_position_t;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      template< typename S, typename... Ts >
         requires( !std::same_as< input_with_source, std::remove_cvref_t< S > > ) && std::constructible_from< InputSource, S > && std::constructible_from< Input, Ts... >
      explicit input_with_source( S&& s, Ts&&... ts )
         : Input( std::forward< Ts >( ts )... ),
           m_source( std::forward< S >( s ) )
      {}

      [[nodiscard]] auto current_position() const
      {
         return error_position_t( m_source, Input::current_position() );
      }

      [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const
      {
         return error_position_t( m_source, Input::previous_position( saved ) );
      }

      [[nodiscard]] const auto& direct_source() const noexcept
      {
         return m_source;
      }

   protected:
      InputSource m_source;
   };

   template< typename Input >
   class input_with_source< void, void, Input >
      : public Input
   {
   public:
      using Input::Input;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
