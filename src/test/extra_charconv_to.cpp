// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#if !defined( __cpp_exceptions )
#include <iostream>
int main()
{
   std::cout << "Exception support disabled, skipping test..." << std::endl;
}
#else

#include "test.hpp"

#include <string_view>

#include <tao/pegtl/extra/builders.hpp>
#include <tao/pegtl/extra/charconv.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct test_target
   {
      int value = 0;
      int first = 0;
      int second = 0;

      void set_pair( const int in_first, const int in_second ) noexcept
      {
         first = in_first;
         second = in_second;
      }
   };

   struct other_state
   {};

#if defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
   struct floating_target
   {
      float value = 0;
      double first = 0;
      float second = 0;

      void set_pair( const double in_first, const float in_second ) noexcept
      {
         first = in_first;
         second = in_second;
      }
   };

   struct floating_rule
      : from_fchars_nothrow< float >
   {};

   struct floating_void_rule
      : from_fchars_nothrow< void, std::chars_format::scientific >
   {};

   struct floating_throws_rule
      : from_fchars_throws< double >
   {};

   struct floating_pair_rule
      : seq< floating_throws_rule, one< ',' >, floating_void_rule, eof >
   {};
#endif

   struct decimal_rule
      : from_chars_nothrow< int >
   {};

   struct hex_rule
      : from_xchars_nothrow< void >
   {};

   struct overflow_rule
      : from_chars_nothrow< int >
   {};

   struct throws_rule
      : from_chars_throws< int >
   {};

   struct pair_rule
      : seq< decimal_rule, one< ',' >, hex_rule, eof >
   {};

   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template<>
   struct test_action< decimal_rule >
      : from_chars_to< &test_target::value >
   {};

   template<>
   struct test_action< hex_rule >
      : from_chars_to< &test_target::value >
   {};

   template<>
   struct test_action< overflow_rule >
      : from_chars_to< &test_target::value >
   {};

   template<>
   struct test_action< throws_rule >
      : from_chars_to< &test_target::value >
   {};

   template<>
   struct test_action< pair_rule >
      : multi_to< &test_target::set_pair,
                  from_chars_for< decimal_rule >,
                  from_chars_for< hex_rule > >
   {};

#if defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
   template<>
   struct test_action< floating_rule >
      : from_chars_to< &floating_target::value >
   {};

   template<>
   struct test_action< floating_void_rule >
      : from_chars_to< &floating_target::value >
   {};

   template<>
   struct test_action< floating_throws_rule >
      : from_chars_to< &floating_target::first >
   {};

   template<>
   struct test_action< floating_pair_rule >
      : multi_to< &floating_target::set_pair,
                  from_chars_for< floating_throws_rule >,
                  from_chars_for< floating_void_rule > >
   {};
#endif

   void test_decimal()
   {
      other_state other;
      test_target target;
      view_input in( "123" );

      TAO_PEGTL_TEST_ASSERT( parse< decimal_rule, test_action >( in, other, target ) );
      TAO_PEGTL_TEST_ASSERT( target.value == 123 );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

   void test_hex()
   {
      other_state other;
      test_target target;
      view_input in( "ff" );

      TAO_PEGTL_TEST_ASSERT( parse< hex_rule, test_action >( in, other, target ) );
      TAO_PEGTL_TEST_ASSERT( target.value == 255 );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

   void test_local_failure()
   {
      other_state other;
      test_target target;
      view_input in( "999999999999" );

      TAO_PEGTL_TEST_ASSERT( !parse< overflow_rule, test_action >( in, other, target ) );
      TAO_PEGTL_TEST_ASSERT( target.value == 0 );
      TAO_PEGTL_TEST_ASSERT( in.size() == 12 );
   }

   void test_global_failure()
   {
      other_state other;
      test_target target;
      view_input in( "999999999999" );

      TAO_PEGTL_TEST_THROWS( parse< throws_rule, test_action >( in, other, target ) );
      TAO_PEGTL_TEST_ASSERT( target.value == 0 );
   }

   void test_for()
   {
      other_state other;
      test_target target;
      view_input in( "123,ff" );

      TAO_PEGTL_TEST_ASSERT( parse< pair_rule, test_action >( in, other, target ) );
      TAO_PEGTL_TEST_ASSERT( target.first == 123 );
      TAO_PEGTL_TEST_ASSERT( target.second == 255 );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

#if defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
   void test_floating()
   {
      other_state other;
      floating_target target;
      {
         view_input in( "1.25" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_rule, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 1.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "25e-2" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_void_rule, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "1.25,25e-2" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_pair_rule, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.first == 1.25 );
         TAO_PEGTL_TEST_ASSERT( target.second == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      for( const auto* input : { "invalid", "1e9999", "1e-9999" } ) {
         view_input in( std::string_view{ input } );
         TAO_PEGTL_TEST_ASSERT( !parse< floating_rule, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.current() == input );
      }
      {
         view_input in( "1e9999" );
         TAO_PEGTL_TEST_THROWS( parse< floating_throws_rule, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.first == 1.25 );
      }
      {
         view_input in( "2.5" );
         TAO_PEGTL_TEST_ASSERT( parse< disable< floating_rule >, test_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
   }
#endif

   void unit_test()
   {
      test_decimal();
      test_hex();
      test_local_failure();
      test_global_failure();
      test_for();
#if defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
      test_floating();
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
