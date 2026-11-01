// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_EXTRACT_POSITION_HPP
#define TAO_PEGTL_INTERNAL_EXTRACT_POSITION_HPP

#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename C >
   concept has_current_position = requires {
      std::declval< C >().current_position();
   };

   template< typename T >
   [[nodiscard]] decltype( auto ) extract_position( const T& t )
   {
      if constexpr( !has_current_position< T > ) {
         // Inputs must have a current_position() function, hence T is
         // not an input and we assume that it is itself the position.
         return t;
      }
      else if constexpr( !has_current_position< decltype( *t.current() ) > ) {
         // T has a current_position() function, hence we assume it is
         // an input, but T::data_t does not have one. The normal case
         // in which we use the input's position.
         return t.current_position();
      }
      else {
         // T has a current_position() function, hence we assume it is
         // an input, but T::data_t also has one. In this case we use
         // T::data_t's position assuming that T::data_t is a token or
         // similar that already has a position from an earlier parse.
         return t.current()->current_position();
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
