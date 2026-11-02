// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string_view>

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   using source_grammar = seq< string< "foo" >, eof >;

   template< typename Source >
   void test_source_success( const Source& name )
   {
      text_view_input< scan::lf, char, Source > in( name, "bar" );
      TAO_PEGTL_TEST_ASSERT( parse< source< source_grammar > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.size() == 3 );
   }

   template< typename Source >
   void test_source_failure( const Source& name )
   {
      text_view_input< scan::lf, char, Source > in( name, "foo" );
      TAO_PEGTL_TEST_ASSERT( !parse< source< source_grammar > >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.size() == 3 );
   }

   void unit_test()
   {
      verify_meta< source< source_grammar >, internal::source< source_grammar > >();
      verify_analyze< source< source_grammar > >( __LINE__, __FILE__, false, false );

      text_view_input< scan::lf, char, std::string_view > enabled_in( "foo", "bar" );
      verify_ctrl_enabled< source< source_grammar > >( __LINE__, __FILE__, enabled_in );
      text_view_input< scan::lf, char, std::string_view > disabled_in( "foo", "bar" );
      verify_ctrl_disabled< internal::source< source_grammar > >( __LINE__, __FILE__, disabled_in );

      test_source_success< std::string >( "foo" );
      test_source_success< std::string_view >( "foo" );

      test_source_failure< std::string >( "bar" );
      test_source_failure< std::string_view >( "bar" );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
