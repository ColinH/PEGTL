// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_HAS_EOL_CHAR_RULE_HPP
#define TAO_PEGTL_INTERNAL_HAS_EOL_CHAR_RULE_HPP

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   concept has_eol_char_rule = requires { typename T::eol_char_rule; };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
