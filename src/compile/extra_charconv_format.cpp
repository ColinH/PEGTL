// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/extra/charconv.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

#if TAO_PEGTL_COMPILE_ACCEPT
using rule = pegtl::from_fchars_nothrow< float, std::chars_format::hex >;
#else
// Only the four enumerators of std::chars_format are supported.
using rule = pegtl::from_fchars_nothrow< void, static_cast< std::chars_format >( 0 ) >;
#endif

int main()
{
   ( void )sizeof( rule );
   return 0;
}
