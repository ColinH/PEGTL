// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_HAS_DIRECT_READER_HPP
#define TAO_PEGTL_STREAM_HAS_DIRECT_READER_HPP

#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename C >
   concept has_direct_reader = requires {
      std::declval< C >().direct_reader();
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

// FCOV_EXCL_FILE
#endif
