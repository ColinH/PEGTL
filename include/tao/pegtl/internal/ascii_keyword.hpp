// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_ASCII_KEYWORD_HPP
#define TAO_PEGTL_INTERNAL_ASCII_KEYWORD_HPP

#include "../config.hpp"

#include "ascii_string.hpp"
#include "identifier.hpp"
#include "not_at.hpp"
#include "peek_direct.hpp"
#include "seq.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< char... Cs >
   struct ascii_keyword
      : seq< ascii_string< Cs... >, not_at< identifier_other< peek_char > > >
   {
      static_assert( sizeof...( Cs ) > 0 );
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
