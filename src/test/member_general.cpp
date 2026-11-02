// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <vector>

#include "test.hpp"
#include "verify_meta.hpp"

#include <tao/pegtl/member.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct member_token
   {
      int value;
      std::string text;
   };

   using member_input = view_input< void, member_token >;
   using value_peek = internal::peek_member< &member_token::value >;

   template< typename Rule >
   void verify_member_data( const std::size_t line, const char* file, const std::vector< member_token >& data, const bool expected, const std::size_t remain )
   {
      member_input in( data );
      const auto result = parse< Rule >( in );

      if( result != expected ) {
         TAO_PEGTL_TEST_FAILED( "result received/expected [ " << result << " / " << expected << " ]" );
      }
      if( in.size() != remain ) {
         TAO_PEGTL_TEST_FAILED( "remain received/expected [ " << in.size() << " / " << remain << " ]" );
      }
   }

   template< typename Rule >
   void verify_member( const std::size_t line, const char* file, const std::vector< int >& values, const bool expected, const std::size_t remain )
   {
      std::vector< member_token > data;
      data.reserve( values.size() );
      for( const int value : values ) {
         data.push_back( { value, std::to_string( value ) } );
      }
      verify_member_data< Rule >( line, file, data, expected, remain );
   }

   bool called = false;
   int seen_value = 0;

   [[nodiscard]] bool is_even( const int value ) noexcept
   {
      called = true;
      seen_value = value;
      return ( value % 2 ) == 0;
   }

   [[nodiscard]] bool is_state_value( const int& value, int& state ) noexcept
   {
      called = true;
      seen_value = value;
      return value == state;
   }

   void test_function()
   {
      verify_meta< member::function< &member_token::value, is_even >, internal::function< value_peek, is_even > >();

      called = false;
      seen_value = 0;
      verify_member< member::function< &member_token::value, is_even > >( __LINE__, __FILE__, { 2 }, true, 0 );
      TAO_PEGTL_TEST_ASSERT( called );
      TAO_PEGTL_TEST_ASSERT( seen_value == 2 );

      called = false;
      seen_value = 0;
      verify_member< member::function< &member_token::value, is_even > >( __LINE__, __FILE__, { 3 }, false, 1 );
      TAO_PEGTL_TEST_ASSERT( called );
      TAO_PEGTL_TEST_ASSERT( seen_value == 3 );

      called = false;
      verify_member< member::function< &member_token::value, is_even > >( __LINE__, __FILE__, {}, false, 0 );
      TAO_PEGTL_TEST_ASSERT( !called );

      int state = 7;
      called = false;
      seen_value = 0;
      const std::vector< member_token > data = {
         { 7, "seven" }
      };
      member_input in( data );
      TAO_PEGTL_TEST_ASSERT( parse< member::function< &member_token::value, is_state_value > >( in, state ) );
      TAO_PEGTL_TEST_ASSERT( called );
      TAO_PEGTL_TEST_ASSERT( seen_value == 7 );
      TAO_PEGTL_TEST_ASSERT( in.size() == 0 );

      verify_member< member::function< &member_token::value, []( const int value ) noexcept { return value == 42; } > >( __LINE__, __FILE__, { 42 }, true, 0 );
      verify_member_data< member::function< &member_token::text, &std::string::empty > >( __LINE__, __FILE__, {
                                                                                                                 { 1, "" }
      },
                                                                                          true,
                                                                                          0 );
      verify_member_data< member::function< &member_token::text, &std::string::empty > >( __LINE__, __FILE__, {
                                                                                                                 { 1, "nonempty" }
      },
                                                                                          false,
                                                                                          1 );
   }

   void test_one()
   {
      verify_meta< member::one< &member_token::value, 1, 3 >, internal::one< value_peek, 1, 3 > >();

      verify_member< member::one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 1 }, true, 0 );
      verify_member< member::one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 3, 2 }, true, 1 );
      verify_member< member::one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 2 }, false, 1 );
   }

   void test_not_one()
   {
      verify_meta< member::not_one< &member_token::value, 1, 3 >, internal::not_one< value_peek, 1, 3 > >();

      verify_member< member::not_one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::not_one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 2 }, true, 0 );
      verify_member< member::not_one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 4, 1 }, true, 1 );
      verify_member< member::not_one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 1 }, false, 1 );
      verify_member< member::not_one< &member_token::value, 1, 3 > >( __LINE__, __FILE__, { 3 }, false, 1 );
   }

   void test_range()
   {
      verify_meta< member::range< &member_token::value, 2, 4 >, internal::range< value_peek, 2, 4 > >();

      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 1 }, false, 1 );
      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 2 }, true, 0 );
      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 3 }, true, 0 );
      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 4 }, true, 0 );
      verify_member< member::range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 5 }, false, 1 );
   }

   void test_not_range()
   {
      verify_meta< member::not_range< &member_token::value, 2, 4 >, internal::not_range< value_peek, 2, 4 > >();

      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 1 }, true, 0 );
      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 2 }, false, 1 );
      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 3 }, false, 1 );
      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 4 }, false, 1 );
      verify_member< member::not_range< &member_token::value, 2, 4 > >( __LINE__, __FILE__, { 5 }, true, 0 );
   }

   void test_ranges()
   {
      verify_meta< member::ranges< &member_token::value, 1, 3, 7, 9 >, internal::ranges< value_peek, 1, 3, 7, 9 > >();

      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 0 }, false, 1 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 1 }, true, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 2 }, true, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 3 }, true, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 5 }, false, 1 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 7 }, true, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 9, 10 }, true, 1 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7 > >( __LINE__, __FILE__, { 7 }, true, 0 );
      verify_member< member::ranges< &member_token::value, 1, 3, 7 > >( __LINE__, __FILE__, { 8 }, false, 1 );
   }

   void test_not_ranges()
   {
      verify_meta< member::not_ranges< &member_token::value, 1, 3, 7, 9 >, internal::not_ranges< value_peek, 1, 3, 7, 9 > >();

      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 0 }, true, 0 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 1 }, false, 1 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 2 }, false, 1 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 3 }, false, 1 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 5 }, true, 0 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 7 }, false, 1 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7, 9 > >( __LINE__, __FILE__, { 9, 10 }, false, 2 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7 > >( __LINE__, __FILE__, { 7 }, false, 1 );
      verify_member< member::not_ranges< &member_token::value, 1, 3, 7 > >( __LINE__, __FILE__, { 8 }, true, 0 );
   }

   void test_string()
   {
      verify_meta< member::string< &member_token::value, 1, 2, 3 >, internal::seq< internal::one< value_peek, 1 >, internal::one< value_peek, 2 >, internal::one< value_peek, 3 > >, internal::one< value_peek, 1 >, internal::one< value_peek, 2 >, internal::one< value_peek, 3 > >();

      verify_member< member::string< &member_token::value, 1, 2, 3 > >( __LINE__, __FILE__, {}, false, 0 );
      verify_member< member::string< &member_token::value, 1, 2, 3 > >( __LINE__, __FILE__, { 0, 2, 3 }, false, 3 );
      verify_member< member::string< &member_token::value, 1, 2, 3 > >( __LINE__, __FILE__, { 1, 2, 3 }, true, 0 );
      verify_member< member::string< &member_token::value, 1, 2, 3 > >( __LINE__, __FILE__, { 1, 2, 3, 4 }, true, 1 );
   }

   void unit_test()
   {
      test_function();
      test_one();
      test_not_one();
      test_range();
      test_not_range();
      test_ranges();
      test_not_ranges();
      test_string();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
