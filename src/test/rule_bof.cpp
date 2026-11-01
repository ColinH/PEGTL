// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

#include <tao/pegtl/stream/inputs.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Input >
   void verify_bof_input()
   {
      Input in( "a" );
      TAO_PEGTL_TEST_ASSERT( in.size( 1 ) == 1 );
      TAO_PEGTL_TEST_ASSERT( parse< bof >( in ) );
      in.template consume< alpha >( 1 );
      TAO_PEGTL_TEST_ASSERT( !parse< bof >( in ) );
   }

   void unit_test()
   {
      verify_meta< bof, internal::bof >();

      verify_ctrl_enabled< bof >( __LINE__, __FILE__, "" );
      verify_ctrl_disabled< internal::bof >( __LINE__, __FILE__, "" );

      verify_analyze< bof >( __LINE__, __FILE__, false, false );

      verify_bof_input< view_input<> >();
      verify_bof_input< array_cstring_input<> >();

      verify_rule< bof >( __LINE__, __FILE__, "", result_type::success, 0 );

      for( char i = 1; i < 127; ++i ) {
         const char s[] = { i, 0 };
         verify_rule< bof >( __LINE__, __FILE__, s, result_type::success, 1 );
      }
      verify_rule< seq< alpha, bof > >( __LINE__, __FILE__, "a", result_type::local_failure, 1 );
      verify_rule< seq< alpha, bof > >( __LINE__, __FILE__, "ab", result_type::local_failure, 2 );
      verify_rule< seq< alpha, bof, alpha > >( __LINE__, __FILE__, "ab", result_type::local_failure, 2 );
      verify_rule< seq< alpha, eol, bof > >( __LINE__, __FILE__, "a\n", result_type::local_failure, 2 );
      verify_rule< seq< alpha, eol, bof > >( __LINE__, __FILE__, "a\nb", result_type::local_failure, 3 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
