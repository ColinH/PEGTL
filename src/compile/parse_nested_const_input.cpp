// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <utility>

#include <tao/pegtl.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

int main()
{
   pegtl::text_view_input ambient( "" );
   pegtl::text_view_input in( "" );
#if TAO_PEGTL_COMPILE_ACCEPT
   return pegtl::parse_nested< pegtl::eof >( ambient, std::move( in ) ) ? 0 : 1;
#else
   const auto const_in = std::move( in );
   // include/tao/pegtl/parse_nested.hpp
   // A const lvalue must be rejected even when the rule does not consume input.
   return pegtl::parse_nested< pegtl::eof >( ambient, const_in ) ? 0 : 1;
#endif
}
