// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <utility>

#include "test.hpp"

#include "endless_stream.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct nop_action
   {
      template< typename ActionInput >
      static void apply( const ActionInput& /*unused*/ ) noexcept
      {}
   };

   void test_single()
   {
      array_endless_input< void > in( 'a' );
      TAO_PEGTL_TEST_ASSERT( in.buffer_capacity() < 10000 );

      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 3000, one< 'a' > > >( in ) );
         TAO_PEGTL_TEST_ASSERT( parse< discard >( in ) );
      }
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 12000, one< 'a' > > >( in ) );
#endif
   }

   void test_single_auto()
   {
      array_endless_auto_input< void > in( 'a' );
      TAO_PEGTL_TEST_ASSERT( in.buffer_capacity() < 10000 );
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );

      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 3000, one< 'a' > > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      }
      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 3000, one< 'a' > >, nothing, normal, apply_mode::enabled, rewind_mode::required >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      }
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 12000, one< 'a' > >, nothing, normal, apply_mode::enabled, rewind_mode::required >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      TAO_PEGTL_TEST_ASSERT( parse< rep< 12000, one< 'a' > > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< at< rep< 12000, one< 'a' > > > >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 12000, one< 'a' > >, nop_action >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
   }

   void test_multiple()
   {
      array_endless_input< void > in( "abc" );
      TAO_PEGTL_TEST_ASSERT( parse< not_at< eof > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.buffer_capacity() < 10000 );

      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 1000, string< "abc" > > >( in ) );
         TAO_PEGTL_TEST_ASSERT( parse< discard >( in ) );
      }
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 4000, string< "abc" > > >( in ) );
#endif
   }

   void test_multiple_auto()
   {
      array_endless_auto_input< void > in( "abc" );
      TAO_PEGTL_TEST_ASSERT( parse< not_at< eof > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.buffer_capacity() < 10000 );
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );

      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 1000, string< "abc" > > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      }
      for( std::size_t i = 0; i < 42; ++i ) {
         TAO_PEGTL_TEST_ASSERT( parse< rep< 1000, string< "abc" > >, nothing, normal, apply_mode::enabled, rewind_mode::required >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      }
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 4000, string< "abc" > >, nothing, normal, apply_mode::enabled, rewind_mode::required >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
      TAO_PEGTL_TEST_ASSERT( parse< rep< 4000, string< "abc" > > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< at< rep< 4000, string< "abc" > > > >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse< rep< 4000, string< "abc" > >, nop_action >( in ) );
#endif
      TAO_PEGTL_TEST_ASSERT( in.get_rewind_guards() == 0 );
   }

   void unit_test()
   {
      test_single();
      test_single_auto();
      test_multiple();
      test_multiple_auto();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
