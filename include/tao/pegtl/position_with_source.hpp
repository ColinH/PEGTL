// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_POSITION_WITH_SOURCE_HPP
#define TAO_PEGTL_POSITION_WITH_SOURCE_HPP

#include <concepts>
#include <ostream>
#include <type_traits>
#include <utility>

#include "config.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Source, typename Position >
   struct position_with_source
      : Position
   {
      Source source = Source();

      static_assert( std::is_same_v< Source, std::decay_t< Source > > );

      position_with_source() noexcept = default;

      // position_with_source( position_with_source&& ) noexcept = default;
      // position_with_source( const position_with_source& ) = default;

      // position_with_source& operator=( position_with_source&& ) noexcept = default;
      // position_with_source& operator=( const position_with_source& ) = default;

      explicit position_with_source( Source&& s )
         : source( std::move( s ) )
      {}

      explicit position_with_source( const Source& s )
         requires std::constructible_from< Source, const Source& >
         : source( s )
      {}

      template< typename S, typename P >
         requires std::constructible_from< Source, S > && std::constructible_from< Position, P >
      position_with_source( S&& s, P&& p )
         : Position( std::forward< P >( p ) ),
           source( std::forward< S >( s ) )
      {}

      // TODO: Why does GCC choose this templated c'tor over the copy c'tor?
      // template< typename S, typename... Ps >
      // explicit position_with_source( S&& s, Ps&&... ps )
      //    : Position( std::forward< Ps >( ps )... ),
      //      source( std::forward< S >( s ) )
      // {}

      [[nodiscard]] Position& base() noexcept
      {
         return *this;
      }

      [[nodiscard]] const Position& base() const noexcept
      {
         return *this;
      }
   };

   template< typename Source, typename Position >
   std::ostream& operator<<( std::ostream& os, const position_with_source< Source, Position >& p )
   {
      return os << p.source << '@' << p.base();
   }

   template< typename Source, typename Position >
   [[nodiscard]] bool operator==( const position_with_source< Source, Position >& l, const position_with_source< Source, Position >& r ) noexcept
   {
      return ( l.source == r.source ) && ( l.base() == r.base() );
   }

   template< typename Source, typename Position >
   [[nodiscard]] bool operator!=( const position_with_source< Source, Position >& l, const position_with_source< Source, Position >& r ) noexcept
   {
      return !( l == r );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif
