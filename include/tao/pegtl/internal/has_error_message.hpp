// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_HAS_ERROR_MESSAGE_HPP
#define TAO_PEGTL_INTERNAL_HAS_ERROR_MESSAGE_HPP

#include <concepts>
#include <type_traits>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule, typename Result = const char* >
   concept has_error_message = std::same_as< std::decay_t< decltype( Rule::error_message ) >, Result >;

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
