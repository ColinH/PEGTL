// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

#include "endless_stream.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      verify_meta< discard, discard >();

      verify_analyze< discard >( __LINE__, __FILE__, false, false );

      verify_rule< discard >( __LINE__, __FILE__, "", result_type::success, 0 );

      for( char i = 1; i < 127; ++i ) {
         char t[] = { i, 0 };
         verify_rule< discard >( __LINE__, __FILE__, std::string( t ), result_type::success, 1 );
      }
      alloc_endless_input< void > dei( 100, 20, "abcdefghijklmnopqrstuvwxyz" );

      TAO_PEGTL_TEST_ASSERT( dei.buffer_used_size() == 0 );
      TAO_PEGTL_TEST_ASSERT( parse< ascii::string< "abc" > >( dei ) );
      TAO_PEGTL_TEST_ASSERT( dei.buffer_used_size() == 17 );
      TAO_PEGTL_TEST_ASSERT( dei.current() == dei.buffer_start() + 3 );
      TAO_PEGTL_TEST_ASSERT( parse< discard >( dei ) );
      TAO_PEGTL_TEST_ASSERT( dei.buffer_used_size() == 17 );
      TAO_PEGTL_TEST_ASSERT( dei.current() == dei.buffer_start() );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
