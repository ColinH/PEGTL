// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <string_view>

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      verify_meta< aistring< 1, "ab" >, internal::ascii_aistring< 1, 'a', 'b' > >();
      verify_meta< aistring< 2, "a0b" >, internal::ascii_aistring< 2, 'a', '0', 'b' > >();
      verify_meta< aistring< 3, "infinity" >, internal::ascii_aistring< 3, 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' > >();
      verify_meta< aistring< 1, "\0\n" >, internal::ascii_aistring< 1, '\0', '\n' > >();

      verify_ctrl_enabled< aistring< 3, "infinity" > >( __LINE__, __FILE__, "INFINITY" );
      verify_ctrl_disabled< internal::ascii_aistring< 3, 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' > >( __LINE__, __FILE__, "INFINITY" );
      verify_ctrl_enabled< aistring< 1, "\0\n" > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );
      verify_ctrl_disabled< internal::ascii_aistring< 1, '\0', '\n' > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );

      verify_analyze< aistring< 1, "ab" > >( __LINE__, __FILE__, true, false );
      verify_analyze< aistring< 2, "a0b" > >( __LINE__, __FILE__, true, false );
      verify_analyze< aistring< 3, "infinity" > >( __LINE__, __FILE__, true, false );
      verify_analyze< aistring< 1, "\0\n" > >( __LINE__, __FILE__, true, false );

      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "", result_type::local_failure );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "i", result_type::local_failure );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "IN", result_type::local_failure );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "ixf", result_type::local_failure );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "nan", result_type::local_failure );

      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "inf", result_type::success, 0 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "INF", result_type::success, 0 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "INFI", result_type::success, 1 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "InFiNiXy", result_type::success, 5 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "infinity", result_type::success, 0 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "INFINITY", result_type::success, 0 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "Infinity+", result_type::success, 1 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "Infinityi", result_type::success, 1 );
      verify_rule< aistring< 3, "infinity" > >( __LINE__, __FILE__, "Infinityinf", result_type::success, 3 );

      verify_rule< aistring< 1, "ab" > >( __LINE__, __FILE__, "a", result_type::success, 0 );
      verify_rule< aistring< 1, "ab" > >( __LINE__, __FILE__, "A", result_type::success, 0 );
      verify_rule< aistring< 1, "ab" > >( __LINE__, __FILE__, "Ab", result_type::success, 0 );
      verify_rule< aistring< 1, "ab" > >( __LINE__, __FILE__, "Ax", result_type::success, 1 );
      verify_rule< aistring< 2, "a0b" > >( __LINE__, __FILE__, "A", result_type::local_failure );
      verify_rule< aistring< 2, "a0b" > >( __LINE__, __FILE__, "AO", result_type::local_failure );
      verify_rule< aistring< 2, "a0b" > >( __LINE__, __FILE__, "A0", result_type::success, 0 );
      verify_rule< aistring< 2, "a0b" > >( __LINE__, __FILE__, "A0x", result_type::success, 1 );
      verify_rule< aistring< 2, "a0b" > >( __LINE__, __FILE__, "A0B", result_type::success, 0 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
