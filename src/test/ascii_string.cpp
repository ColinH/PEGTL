// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      verify_meta< string< "" >, internal::success >();
      verify_meta< string< "a" >, internal::one< internal::peek_char, 'a' > >();
      verify_meta< string< "ab" >, internal::ascii_string< 'a', 'b' > >();
      verify_meta< string< "\0\n" >, internal::ascii_string< '\0', '\n' > >();

      verify_ctrl_enabled< string< "" > >( __LINE__, __FILE__, "" );
      verify_ctrl_disabled< internal::ascii_string<> >( __LINE__, __FILE__, "" );
      verify_ctrl_enabled< string< "a" > >( __LINE__, __FILE__, "a" );
      verify_ctrl_disabled< internal::one< internal::peek_char, 'a' > >( __LINE__, __FILE__, "a" );
      verify_ctrl_disabled< internal::terminal< internal::one< internal::peek_char, 'a' >, internal::peek_char > >( __LINE__, __FILE__, "a" );
      verify_ctrl_enabled< string< "ab" > >( __LINE__, __FILE__, "ab" );
      verify_ctrl_disabled< internal::ascii_string< 'a', 'b' > >( __LINE__, __FILE__, "ab" );
      verify_ctrl_enabled< string< "\0\n" > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );
      verify_ctrl_disabled< internal::ascii_string< '\0', '\n' > >( __LINE__, __FILE__, std::string_view( "\0\n", 2 ) );

      verify_analyze< string< "" > >( __LINE__, __FILE__, false, false );
      verify_analyze< string_chars< 1 > >( __LINE__, __FILE__, true, false );
      verify_analyze< string_chars< 1, 2 > >( __LINE__, __FILE__, true, false );
      verify_analyze< string_chars< 1, 2, 3, 4 > >( __LINE__, __FILE__, true, false );
      verify_analyze< string_chars< 1, 2, 3, 4, 5, 6, 7 > >( __LINE__, __FILE__, true, false );

      verify_rule< string< "" > >( __LINE__, __FILE__, "", result_type::success, 0 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "", result_type::local_failure, 0 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "a", result_type::local_failure, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "b", result_type::local_failure, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "c", result_type::local_failure, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "aa", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "aB", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "AB", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "Ab", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "ac", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "ba", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "bb", result_type::local_failure, 2 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "aab", result_type::local_failure, 3 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "aab", result_type::local_failure, 3 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "ab", result_type::success, 0 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "aba", result_type::success, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "abb", result_type::success, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "abc", result_type::success, 1 );
      verify_rule< string< "ab" > >( __LINE__, __FILE__, "abab", result_type::success, 2 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
