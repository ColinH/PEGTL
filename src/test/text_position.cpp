// Copyright (c) 2016-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#if !defined( __cpp_exceptions ) || !defined( __cpp_rtti )
#include <iostream>
int main()
{
   std::cout << "Exception and/or RTTI support disabled, skipping test..." << std::endl;
}
#elif defined( __clang__ ) && defined( _WIN32 )
#include <iostream>
int main()
{
   std::cout << "Nested exceptions are broken with Clang on Windows, skipping test..." << std::endl;
}
#else

#include "test.hpp"

#include <tao/pegtl/extra/nested_exceptions.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule, typename ParseInput = text_view_input< scan::lf_crlf, char, std::string > >
   void test_matches_lf()
   {
      static const std::string s1 = "\n";

      ParseInput i1( __FUNCTION__, s1 );

      TAO_PEGTL_TEST_ASSERT( parse< Rule >( i1 ) );
      TAO_PEGTL_TEST_ASSERT( i1.direct_line() == 2 );
      TAO_PEGTL_TEST_ASSERT( i1.direct_column() == 1 );
   }

   template< typename Rule, typename ParseInput = text_view_input< scan::lf_crlf, char, std::string > >
   void test_matches_other( const std::string& s2 )
   {
      TAO_PEGTL_TEST_ASSERT( s2.size() == 1 );

      ParseInput i2( __FUNCTION__, s2 );

      TAO_PEGTL_TEST_ASSERT( parse< Rule >( i2 ) );
      TAO_PEGTL_TEST_ASSERT( i2.direct_line() == 1 );
      TAO_PEGTL_TEST_ASSERT( i2.direct_column() == 2 );
   }

   template< typename Rule, typename ParseInput = text_view_input< scan::lf_crlf, char, std::string > >
   void test_mismatch( const std::string& s3 )
   {
      TAO_PEGTL_TEST_ASSERT( s3.size() == 1 );

      ParseInput i3( __FUNCTION__, s3 );

      TAO_PEGTL_TEST_ASSERT( !parse< Rule >( i3 ) );
      TAO_PEGTL_TEST_ASSERT( i3.direct_line() == 1 );
      TAO_PEGTL_TEST_ASSERT( i3.direct_column() == 1 );
   }

   struct outer_grammar
      : must< two< 'a' >, two< 'b' >, two< 'c' >, eof >
   {};

   struct inner_grammar
      : must< one< 'd' >, two< 'e' >, eof >
   {};

   template< typename Rule >
   struct outer_action
   {};

   template<>
   struct outer_action< two< 'b' > >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& oi, const bool mode )
      {
         const auto p = oi.current_position();
         TAO_PEGTL_TEST_ASSERT( p.source == "outer" );
         TAO_PEGTL_TEST_ASSERT( p.count == 2 );
         TAO_PEGTL_TEST_ASSERT( p.line == 1 );
         TAO_PEGTL_TEST_ASSERT( p.column == 3 );
         text_view_input< scan::lf_crlf, char, std::string > in( "inner", "dFF" );
         if( mode ) {
            parse_nested< inner_grammar >( oi, in );
         }
         else {
            parse_nested< inner_grammar >( oi.current_position(), in );
         }
      }
   };

   void test_nested_asserts()
   {
      const std::vector< parse_error_base > errors = flatten_base();
      TAO_PEGTL_TEST_ASSERT( errors.size() == 2 );
      TAO_PEGTL_TEST_ASSERT( errors[ 0 ].position_string() == "inner@1:2(1)" );
      TAO_PEGTL_TEST_ASSERT( errors[ 1 ].position_string() == "outer@1:3(2)" );
   }

   template< typename ParseInput >
   void test_nested()
   {
      try {
         ParseInput oi( "outer", "aabbcc" );
         parse< outer_grammar, outer_action >( oi, true );
      }
      catch( ... ) {
         test_nested_asserts();
      }
      try {
         ParseInput oi( "outer", "aabbcc" );
         parse< outer_grammar, outer_action >( oi, false );
      }
      catch( ... ) {
         test_nested_asserts();
      }
   }

   template< typename Rule, typename ParseInput >
   void test_until_position( const std::string& data, const std::size_t remain, const std::size_t line, const std::size_t column, const std::size_t count )
   {
      ParseInput in( "until", data );

      TAO_PEGTL_TEST_ASSERT( parse< Rule >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.size() == remain );

      const auto p = in.current_position();
      TAO_PEGTL_TEST_ASSERT( p.source == "until" );
      TAO_PEGTL_TEST_ASSERT( p.line == line );
      TAO_PEGTL_TEST_ASSERT( p.column == column );
      TAO_PEGTL_TEST_ASSERT( p.count == count );
   }

   template< typename ParseInput >
   void test_until()
   {
      test_until_position< until< eol >, ParseInput >( "ab\n", 0, 2, 1, 3 );
      test_until_position< until< eol >, ParseInput >( "ab\r\n", 0, 2, 1, 4 );

      test_until_position< until< at< eol > >, ParseInput >( "ab\ncd", 3, 1, 3, 2 );
      test_until_position< until< at< eol > >, ParseInput >( "ab\r\ncd", 4, 1, 3, 2 );

      test_until_position< until< eolf >, ParseInput >( "ab", 0, 1, 3, 2 );
      test_until_position< until< at< one< 'b' > > >, ParseInput >( "a\nb", 1, 2, 1, 2 );
   }

   void unit_test()
   {
      test_matches_lf< any >();
      test_matches_other< any >( " " );

      test_matches_lf< one< '\n' > >();
      test_mismatch< one< '\n' > >( " " );

      test_matches_lf< one< ' ', '\n' > >();
      test_matches_other< one< ' ', '\n' > >( " " );

      test_matches_lf< one< ' ', '\n', 'b' > >();
      test_matches_other< one< ' ', '\n', 'b' > >( " " );

      test_matches_lf< string< "\n" > >();
      test_mismatch< string< "\n" > >( " " );

      test_matches_other< string< " " > >( " " );
      test_mismatch< string< " " > >( "\n" );

      test_matches_lf< range< 8, 33 > >();
      test_matches_other< range< 8, 33 > >( " " );

      test_mismatch< range< 11, 30 > >( "\n" );
      test_mismatch< range< 11, 30 > >( " " );

      test_matches_lf< not_range< 20, 30 > >();
      test_matches_other< not_range< 20, 30 > >( " " );

      test_mismatch< not_range< 5, 35 > >( "\n" );
      test_mismatch< not_range< 5, 35 > >( " " );

      test_matches_lf< ranges< 'a', 'z', 8, 33, 'A', 'Z' > >();
      test_matches_other< ranges< 'a', 'z', 8, 33, 'A', 'Z' > >( "N" );
      test_mismatch< ranges< 'a', 'z', 8, 33, 'A', 'Z' > >( "9" );

      test_matches_lf< ranges< 'a', 'z', 'A', 'Z', '\n' > >();
      test_matches_other< ranges< 'a', 'z', 'A', 'Z', '\n' > >( "P" );
      test_mismatch< ranges< 'a', 'z', 'A', 'Z', '\n' > >( "8" );

      test_nested< text_view_input< lazy::lf_crlf, char, std::string > >();
      test_nested< text_view_input< scan::lf_crlf, char, std::string > >();

      test_until< text_view_input< lazy::lf_crlf, char, std::string > >();
      test_until< text_view_input< scan::lf_crlf, char, std::string > >();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
