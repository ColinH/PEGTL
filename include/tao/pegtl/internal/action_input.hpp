// Copyright (c) 2016-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_ACTION_INPUT_HPP
#define TAO_PEGTL_INTERNAL_ACTION_INPUT_HPP

#include <cstddef>

#include "../config.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename ParseInput >
   class action_input
   {
   public:
      using data_t = ParseInput::data_t;
      using input_t = ParseInput;
      using error_position_t = ParseInput::error_position_t;
      using rewind_position_t = ParseInput::rewind_position_t;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      action_input( const rewind_position_t& begin, const ParseInput& input ) noexcept
         : m_saved( begin ),
           m_input( input )
      {}

      action_input( action_input&& ) = delete;
      action_input( const action_input& ) = delete;

      ~action_input() = default;

      void operator=( action_input&& ) = delete;
      void operator=( const action_input& ) = delete;

      [[nodiscard]] const data_t* begin() const noexcept
      {
         return m_input.previous( m_saved );
      }

      [[nodiscard]] const data_t* current( const std::size_t offset = 0 ) const noexcept
      {
         return m_input.previous( m_saved ) + offset;
      }

      [[nodiscard]] const data_t* end() const noexcept
      {
         return m_input.current();
      }

      [[nodiscard]] bool empty() const noexcept
      {
         return begin() == end();
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return std::size_t( end() - begin() );
      }

      [[nodiscard]] std::ptrdiff_t ssize() const noexcept
      {
         return end() - begin();
      }

      [[nodiscard]] const ParseInput& input() const noexcept
      {
         return m_input;
      }

      [[nodiscard]] decltype( auto ) current_position() const
      {
         return m_input.previous_position( m_saved );  // NOTE: O(n) with lazy inputs!
      }

      [[nodiscard]] const rewind_position_t& rewind_position() const noexcept
      {
         return m_saved;
      }

      [[nodiscard]] decltype( auto ) direct_source() const noexcept
      requires requires( const ParseInput& in ) {
         in.direct_source();
      } {
         return m_input.direct_source();  // Not all inputs have this.
      }

      [[nodiscard]] decltype( auto ) direct_position() const noexcept
      requires requires( const ParseInput& in ) {
         in.direct_position();
      } {
         return m_input.direct_position();  // Not all inputs have this.
      }

   protected:
      const rewind_position_t m_saved;
      const ParseInput& m_input;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
