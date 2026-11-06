// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl.hpp>
#include <tao/pegtl/extra/charconv.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

int main()
{
#if TAO_PEGTL_COMPILE_ACCEPT
   float state = 0;
#else
   // A void floating-point rule must deduce float or double from the first state.
   int state = 0;
#endif
   pegtl::view_input in( "1.25" );
   return pegtl::parse< pegtl::from_fchars_nothrow< void > >( in, state ) ? 0 : 1;
}
