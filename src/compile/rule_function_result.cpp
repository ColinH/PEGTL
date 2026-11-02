// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

#if TAO_PEGTL_COMPILE_ACCEPT
constexpr auto predicate = []( auto& ) { return true; };
#else
// include/tao/pegtl/internal/function.hpp
// The callable must return a result convertible to bool.
constexpr auto predicate = []( auto& ) {};
#endif

int main()
{
   pegtl::view_input in( "" );
   return pegtl::parse< pegtl::function< predicate > >( in ) ? 0 : 1;
}
