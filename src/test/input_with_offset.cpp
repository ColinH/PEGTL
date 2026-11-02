// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "test.hpp"
#include "test_utility.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   const std::string data = "foo\nbar";

   using rule1 = seq< string< "foo" >, eol >;
   using rule2 = seq< string< "bar" >, eof >;

   void test_count_offset()
   {
      input_with_offset< copy_input< scan::lf > > in( count_position( 42 ), data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 42 );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 46 );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 46 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 49 );
      TAO_PEGTL_TEST_ASSERT( in.direct_offset() == count_position( 42 ) );
   }

   void test_count_offset_source()
   {
      input_with_offset< copy_input< scan::lf, std::string, const char*, std::string > > in( count_position( 42 ), __FUNCTION__, data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 42 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 46 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 46 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 49 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
   }

   void test_text_offset()
   {
      input_with_offset< text_copy_input< scan::lf > > in( text_position( 200, 300, 100 ), data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 100 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 200 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 300 );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 1 );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 107 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 4 );
      TAO_PEGTL_TEST_ASSERT( in.direct_offset() == text_position( 200, 300, 100 ) );
   }

   void test_text_offset_source()
   {
      input_with_offset< text_copy_input< scan::lf, std::string, std::string, std::string > > in( text_position( 200, 300, 100 ), __FUNCTION__, data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 100 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 200 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 300 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 107 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 4 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
   }

   void test_lazy_offset()
   {
      input_with_offset< text_copy_input< lazy::lf > > in( text_position( 200, 300, 100 ), data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 100 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 200 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 300 );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 1 );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 107 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 4 );
   }

   void test_lazy_offset_source()
   {
      input_with_offset< text_copy_input< lazy::lf, std::string, const char*, std::string > > in( text_position( 200, 300, 100 ), __FUNCTION__, data );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 100 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 200 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 300 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position() == in.previous_position( in.rewind_position() ) );
      const auto a = parse< rule1 >( in );
      TAO_PEGTL_TEST_ASSERT( a );
      const auto r = in.rewind_position();
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
      const auto b = parse< rule2 >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).count == 104 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).column == 1 );
      TAO_PEGTL_TEST_ASSERT( in.previous_position( r ).source == __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( in.current_position().count == 107 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().line == 201 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().column == 4 );
      TAO_PEGTL_TEST_ASSERT( in.current_position().source == __FUNCTION__ );
   }

   void unit_test()
   {
      test_count_offset();
      test_count_offset_source();
      test_text_offset();
      test_text_offset_source();
      test_lazy_offset();
      test_lazy_offset_source();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
