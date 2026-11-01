// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_HAS_EOL_LAZY_PEEK_HPP
#define TAO_PEGTL_INTERNAL_HAS_EOL_LAZY_PEEK_HPP

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   concept has_eol_lazy_peek = requires { typename T::eol_lazy_peek; };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
