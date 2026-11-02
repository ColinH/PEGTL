// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
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
#include "test_utility.hpp"

namespace test2
{
   using namespace TAO_PEGTL_NAMESPACE;

   // clang-format off
   struct a : one< 'a' > {};
   struct b : one< 'b' > {};
   struct grammar : sor< a, sor< b, raise_message< "test123" > > > {};
   // clang-format on

   struct literal_error
   {};

   struct chars_error
   {};

   template< typename Rule >
   struct control
      : normal< Rule >
   {};

   template< auto Text >
   struct control< raise_message< Text > >
      : normal< raise_message< Text > >
   {
      template< typename ParseInput >
      [[noreturn]] static void raise( const ParseInput& /*unused*/ )
      {
         throw literal_error();
      }
   };

   template< char... Cs >
   struct control< raise_message_chars< Cs... > >
      : normal< raise_message_chars< Cs... > >
   {
      template< typename ParseInput >
      [[noreturn]] static void raise( const ParseInput& /*unused*/ )
      {
         throw chars_error();
      }
   };

   template< typename Rule, typename Error >
   void verify_control()
   {
      try {
         parse< Rule, nothing, control >( view_input( "" ) );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const Error& ) {
      }
   }

}  // namespace test2

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      try {
         parse< test2::grammar >( text_view_input< scan::lf >( "c" ) );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const parse_error_base& e ) {
         TAO_PEGTL_TEST_ASSERT( e.message() == "test123" );
      }

      try {
         parse< raise_message_chars< 't', 'e', 's', 't', '1', '2', '3' > >( view_input( "" ) );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const parse_error_base& e ) {
         TAO_PEGTL_TEST_ASSERT( e.message() == "test123" );
      }

      test2::verify_control< raise_message< "error" >, test2::literal_error >();
      test2::verify_control< raise_message_chars< 'e', 'r', 'r', 'o', 'r' >, test2::chars_error >();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
