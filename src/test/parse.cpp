// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <utility>

#include "test.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Input >
   concept can_parse = requires( Input&& in, const int& state ) {
      parse< eof >( std::forward< Input >( in ), state );
   };

   template< typename Input >
   concept can_parse_nested = requires( const count_position& ambient, Input&& in, const int& state ) {
      parse_nested< eof >( ambient, std::forward< Input >( in ), state );
   };

   using input = view_input< scan::lf >;

   static_assert( can_parse< input& > );
   static_assert( can_parse< input > );
   static_assert( !can_parse< const input& > );
   static_assert( !can_parse< const input > );
   static_assert( can_parse_nested< input& > );
   static_assert( can_parse_nested< input > );
   static_assert( !can_parse_nested< const input& > );
   static_assert( !can_parse_nested< const input > );

   void unit_test()
   {
      const int state = 42;
      const count_position ambient( 17 );
      {
         input in( "ab" );
         TAO_PEGTL_TEST_ASSERT( parse< one< 'a' > >( in, state ) );
         TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
         TAO_PEGTL_TEST_ASSERT( parse< seq< one< 'b' >, eof > >( std::move( in ), state ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         input in( "ab" );
         TAO_PEGTL_TEST_ASSERT( parse_nested< one< 'a' > >( ambient, in, state ) );
         TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
         TAO_PEGTL_TEST_ASSERT( parse_nested< seq< one< 'b' >, eof > >( ambient, std::move( in ), state ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      TAO_PEGTL_TEST_ASSERT( parse< seq< one< 'a' >, eof > >( input( "a" ), state ) );
      TAO_PEGTL_TEST_ASSERT( parse_nested< seq< one< 'a' >, eof > >( ambient, input( "a" ), state ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
