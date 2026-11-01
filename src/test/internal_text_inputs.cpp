// Copyright (c) 2021-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "test_utility.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Input >
   concept has_end_of_line = requires( const Input& in, const typename Input::error_position_t& pos ) {
      in.end_of_line_or_file( pos );
   };

   static_assert( has_end_of_line< text_view_input< scan::lf > > );
   static_assert( has_end_of_line< text_view_input< scan::lf, char, std::string > > );
   static_assert( !has_end_of_line< text_view_input< scan::lf, char32_t > > );
   static_assert( !has_end_of_line< text_view_input< scan::lf, char32_t, std::string > > );

   void test_01_text()
   {
      using input_0 = text_view_input< scan::lf >;
      {
         input_0 in( "" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eof >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " " );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( "\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( "\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::any >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( "\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::one< '\n' > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( "\n " );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 2 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " \n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 3 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 3 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " \n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 2 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 2 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 1 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 3 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( "\n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 2 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 2 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 2 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " \n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 1 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 1 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( parse< ascii::many< 2 > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 3 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " \n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 1 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 2 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_0 in( " \n " );
         const char* start = in.current();
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 1 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( parse< one< '\n' > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.current() == start + 2 );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
   }

   void test_02_text()
   {
      using input_1 = text_view_input< ascii::lf >;
      {
         input_1 in( "" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eof >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_1 in( " " );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_1 in( "\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( std::is_same_v< ascii::lf::rule_t, input_1::eol_rule::rule_t > );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      using input_2 = text_view_input< ascii::cr_lf_crlf >;
      {
         input_2 in( "" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eof >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_2 in( " " );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_2 in( "\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
      }
      {
         input_2 in( "\r" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_2 in( "\r\n" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.direct_position(), 2, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_2 in( "\n\r" );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 1 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 3, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
      {
         input_2 in( " \n " );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 1, 0 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 1, 2, 1 ) );
         TAO_PEGTL_TEST_ASSERT( parse< eol >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 1, 2 ) );
         TAO_PEGTL_TEST_ASSERT( parse< blank >( in ) );
         TAO_PEGTL_TEST_ASSERT( test::equal( in.current_position(), 2, 2, 3 ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_position() == in.current_position() );
      }
   }

   void unit_test()
   {
      test_01_text();
      test_02_text();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
