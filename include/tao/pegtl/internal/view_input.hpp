// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_VIEW_INPUT_HPP
#define TAO_PEGTL_INTERNAL_VIEW_INPUT_HPP

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "../config.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif
#include "../pointer_position.hpp"

#include "is_simple_type.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Data >
   class view_input
   {
   public:
      using data_t = Data;
      using error_position_t = pointer_position< data_t >;
      using offset_position_t = void;
      using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      view_input( const data_t* b, const data_t* e ) noexcept
         : m_current( b ),
           m_end( e )
      {}

      view_input( const data_t* b, const std::size_t s ) noexcept
         : view_input( b, b + s )
      {}

      view_input( std::string&& ) = delete;
      view_input( const std::string&& ) = delete;

      explicit view_input( std::string& data ) noexcept
         requires( sizeof( data_t ) == 1 && is_simple_type_v< data_t > )
         : view_input( std::as_const( data ) )
      {}

      explicit view_input( const std::string& data ) noexcept
         requires( sizeof( data_t ) == 1 && is_simple_type_v< data_t > )
         : view_input( reinterpret_cast< const data_t* >( data.data() ), data.size() )
      {}

      explicit view_input( const std::string_view data ) noexcept
         requires( sizeof( data_t ) == 1 && is_simple_type_v< data_t > )
         : view_input( reinterpret_cast< const data_t* >( data.data() ), data.size() )
      {}

      template< std::size_t E >
      explicit view_input( const std::span< const data_t, E > data ) noexcept
         : view_input( data.data(), data.size() )
      {}

      view_input( std::vector< data_t >&& ) = delete;
      view_input( const std::vector< data_t >&& ) = delete;

      explicit view_input( std::vector< data_t >& data ) noexcept
         : view_input( std::as_const( data ) )
      {}

      explicit view_input( const std::vector< data_t >& data ) noexcept
         : view_input( data.data(), data.size() )
      {}

      template< std::size_t N >
         requires( sizeof( data_t ) == 1 && is_simple_type_v< data_t > )
      explicit view_input( const char ( &l )[ N ] ) noexcept
         : view_input( reinterpret_cast< const data_t* >( l ), N - 1 )
      {}

      template< std::size_t Size >
      explicit view_input( const std::array< data_t, Size >& a ) noexcept
         : view_input( a.data(), a.size() )
      {}

      [[nodiscard]] bool empty() const noexcept
      {
         return size() == 0;
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return m_end - m_current;
      }

      [[nodiscard]] const data_t* current( const std::size_t offset = 0 ) const noexcept
      {
         return m_current + offset;
      }

      // [[nodiscard]] const data_t* previous( const error_position_t saved ) const noexcept

      [[nodiscard]] const data_t* previous( const rewind_position_t saved ) const noexcept
      {
         return saved.data;
      }

      [[nodiscard]] const data_t* end() const noexcept
      {
         return m_end;
      }

      template< typename Rule >
      void consume( const std::size_t count ) noexcept
      {
         m_current += count;
      }

      [[nodiscard]] auto rewind_position() const noexcept
      {
         return rewind_position_t( m_current );
      }

      void rewind_to_position( const rewind_position_t saved ) noexcept
      {
         m_current = saved.data;
      }

      [[nodiscard]] auto current_position() const noexcept
      {
         return error_position_t( m_current );
      }

      [[nodiscard]] auto previous_position( const rewind_position_t saved ) const noexcept
      {
         return saved;  // error_position_t
      }

      void private_set_current( const data_t* in_current ) noexcept
      {
         m_current = in_current;
      }

      void private_set_end( const data_t* e ) noexcept
      {
         m_end = e;
      }

   protected:
      const data_t* m_current;
      const data_t* m_end;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
