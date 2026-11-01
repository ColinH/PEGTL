// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_HAS_APPLY_HPP
#define TAO_PEGTL_INTERNAL_HAS_APPLY_HPP

#include <concepts>
#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Control, typename Result, template< typename... > class Action, typename... States >
   concept has_apply = requires {
      { Control::template apply< Action >( std::declval< States >()... ) } -> std::same_as< Result >;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
