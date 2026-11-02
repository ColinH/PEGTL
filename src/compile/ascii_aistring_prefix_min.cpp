// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

#if TAO_PEGTL_COMPILE_ACCEPT
using rule = pegtl::aistring< 1, "ab" >;
#else
// include/tao/pegtl/internal/ascii_aistring.hpp
// static_assert( Prefix > 0 );
using rule = pegtl::aistring< 0, "ab" >;
#endif

int main()
{
   (void)sizeof( rule );
   return 0;
}
