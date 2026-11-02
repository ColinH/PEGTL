// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_SRC_TEST_VERIFY_FILE_HPP
#define TAO_PEGTL_SRC_TEST_VERIFY_FILE_HPP

#include "test_utility.hpp"

#if defined( _MSC_VER ) || defined( __MINGW32__ )
#define TAO_PEGTL_TEST_FILENAME u"src/test/file_äöü𝄞_data.txt"
#else
#define TAO_PEGTL_TEST_FILENAME "src/test/file_äöü𝄞_data.txt"
#endif

namespace TAO_PEGTL_NAMESPACE
{
   struct file_content
      : seq< string< "dummy content" >, eol >
   {};

   struct file_grammar
      : seq< rep_min_max< 11, 11, file_content >, eof >
   {};

   template< typename Rule >
   struct file_action
   {};

   template<>
   struct file_action< eof >
   {
      static void apply0( bool& flag )
      {
         flag = true;
      }
   };

   template< typename Rule >
   struct file_control
      : normal< Rule >
   {};

   template<>
   struct file_control< eof >
      : normal< eof >
   {
      template< typename ParseInput >
      static void success( const ParseInput& /*unused*/, bool& flag )
      {
         flag = true;
      }
   };

   template< typename T >
   void verify_file()
   {
#if defined( __cpp_exceptions )
      {
         try {
            T in( "src/test/no_such_file.txt" );
            TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
         }
         catch( const std::filesystem::filesystem_error& ) {
         }
      }
#endif
      {
         T in( "src/test/data/test_data.txt" );
         TAO_PEGTL_TEST_ASSERT( in.direct_source() == "src/test/data/test_data.txt" );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.direct_source() == "src/test/data/test_data.txt" );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = true;
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in, flag ) );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in, flag ) );
         TAO_PEGTL_TEST_ASSERT( flag == false );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         const bool result = parse< file_grammar, file_action >( in, flag );
         TAO_PEGTL_TEST_ASSERT( result );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         const bool result = parse< file_grammar, nothing, file_control >( in, flag );
         TAO_PEGTL_TEST_ASSERT( result );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
#if defined( __cpp_exceptions )
      const char* foo = "foo";
      const text_copy_input< scan::lf_crlf, std::string, std::string > m( foo, foo, foo + 3 );
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         TAO_PEGTL_TEST_ASSERT( parse_nested< file_grammar >( m, in ) );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = true;
         TAO_PEGTL_TEST_ASSERT( parse_nested< file_grammar >( m, in, flag ) );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         TAO_PEGTL_TEST_ASSERT( parse_nested< file_grammar >( m, in, flag ) );
         TAO_PEGTL_TEST_ASSERT( flag == false );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         const bool result = parse_nested< file_grammar, file_action >( m, in, flag );
         TAO_PEGTL_TEST_ASSERT( result );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
      {
         T in( TAO_PEGTL_TEST_FILENAME );
         bool flag = false;
         const bool result = parse_nested< file_grammar, nothing, file_control >( m, in, flag );
         TAO_PEGTL_TEST_ASSERT( result );
         TAO_PEGTL_TEST_ASSERT( flag == true );
      }
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif
