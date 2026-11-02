// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstring>

#include "test.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      char data[ 12 ];
      std::memcpy( data, "foo\0bar\0baz", 12 );
      char* argv[] = { data, data + 4, data + 8 };
      argv_input< scan::lf > in( argv, 1 );
      TAO_PEGTL_TEST_ASSERT( in.direct_source() == "argv[1]" );
      const bool r1 = parse< string< "bar" > >( in );
      TAO_PEGTL_TEST_ASSERT( r1 );
      argv_input< void, void > i2( argv, 2 );
      const bool r2 = parse< string< "baz" > >( i2 );
      TAO_PEGTL_TEST_ASSERT( r2 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
