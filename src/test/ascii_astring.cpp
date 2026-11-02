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
      verify_meta< astring< 1, "ab" >, internal::ascii_astring< 1, 'a', 'b' > >();
      verify_meta< astring< 2, "abc" >, internal::ascii_astring< 2, 'a', 'b', 'c' > >();
      verify_meta< astring< 3, "infinity" >, internal::ascii_astring< 3, 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' > >();
      verify_meta< astring< 1, "\0\n" >, internal::ascii_astring< 1, '\0', '\n' > >();

      verify_ctrl_enabled< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinity" );
      verify_ctrl_disabled< internal::ascii_astring< 3, 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' > >( __LINE__, __FILE__, "infinity" );
      verify_ctrl_enabled< astring< 1, "\0\n" > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );
      verify_ctrl_disabled< internal::ascii_astring< 1, '\0', '\n' > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );

      verify_analyze< astring< 1, "ab" > >( __LINE__, __FILE__, true, false );
      verify_analyze< astring< 2, "abc" > >( __LINE__, __FILE__, true, false );
      verify_analyze< astring< 3, "infinity" > >( __LINE__, __FILE__, true, false );
      verify_analyze< astring< 1, "\0\n" > >( __LINE__, __FILE__, true, false );

      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "", result_type::local_failure );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "i", result_type::local_failure );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "in", result_type::local_failure );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "ixf", result_type::local_failure );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "INF", result_type::local_failure );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "nan", result_type::local_failure );

      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "inf", result_type::success, 0 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infi", result_type::success, 1 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinix", result_type::success, 4 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinixy", result_type::success, 5 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinity", result_type::success, 0 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinity+", result_type::success, 1 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinityi", result_type::success, 1 );
      verify_rule< astring< 3, "infinity" > >( __LINE__, __FILE__, "infinityinf", result_type::success, 3 );

      verify_rule< astring< 1, "ab" > >( __LINE__, __FILE__, "a", result_type::success, 0 );
      verify_rule< astring< 1, "ab" > >( __LINE__, __FILE__, "ab", result_type::success, 0 );
      verify_rule< astring< 1, "ab" > >( __LINE__, __FILE__, "ax", result_type::success, 1 );
      verify_rule< astring< 1, "ab" > >( __LINE__, __FILE__, "A", result_type::local_failure );
      verify_rule< astring< 2, "abc" > >( __LINE__, __FILE__, "a", result_type::local_failure );
      verify_rule< astring< 2, "abc" > >( __LINE__, __FILE__, "ab", result_type::success, 0 );
      verify_rule< astring< 2, "abc" > >( __LINE__, __FILE__, "abx", result_type::success, 1 );
      verify_rule< astring< 2, "abc" > >( __LINE__, __FILE__, "abc", result_type::success, 0 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
