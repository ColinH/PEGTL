// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "test.hpp"

#include <tao/pegtl/stream.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule, template< typename... > class Action = nothing >
   bool parse_cstring( const char* string, const std::size_t size, const std::size_t chunk )
   {
      alloc_cstring_input< void > in( size, chunk, string );
      return parse< Rule, Action >( in );
   }

   void unit_test()
   {
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, eof > >( "abc", 4, 1 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, eof > >( "abc", 4, 3 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, eof > >( "abc", 8, 1 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, eof > >( "abc", 8, 4 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, eof > >( "abc", 8, 7 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, discard, eof > >( "abc", 3, 1 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, discard, eof > >( "abc", 3, 2 ) );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse_cstring< seq< string< "abc" >, eof > >( "abc", 3, 1 ) );
      TAO_PEGTL_TEST_THROWS( parse_cstring< seq< string< "abc" >, eof > >( "abc", 3, 2 ) );
#endif
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 12, 1 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 12, 2 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 12, 8 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 12, 11 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" >, discard >, eof > >( "abcabcabc", 4, 1 ) );
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< rep< 3, string< "abc" >, discard >, eof > >( "abcabcabc", 4, 2 ) );
#if defined( __cpp_exceptions )
      TAO_PEGTL_TEST_THROWS( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 4, 1 ) );
      TAO_PEGTL_TEST_THROWS( parse_cstring< seq< rep< 3, string< "abc" > >, eof > >( "abcabcabc", 4, 2 ) );
#endif
      TAO_PEGTL_TEST_ASSERT( parse_cstring< seq< string< "abc" >, discard, string< "def" >, eof > >( "abcdef", 4, 1 ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
