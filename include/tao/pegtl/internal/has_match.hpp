// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_HAS_MATCH_HPP
#define TAO_PEGTL_INTERNAL_HAS_MATCH_HPP

#include <utility>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../rewind_mode.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   concept has_match = requires {
      Action< Rule >::template match< Rule, A, M, Action, Control >( std::declval< ParseInput& >(), std::declval< States&& >()... );
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
