// Copyright (c) 2016-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <clocale>
#include <cstdio>
#include <iostream>
#include <string>

#include "test.hpp"

#include <tao/pegtl/stream.hpp>

#include <tao/pegtl/internal/read_file_stdio.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct file_content
      : seq< string< "dummy content" >, eol, discard >
   {};

   struct file_grammar
      : seq< rep_min_max< 11, 11, file_content >, eof >
   {};

   void unit_test()
   {
      const char* const filename = "src/test/data/test_data.txt";
      {
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         const std::string content = internal::read_file_stdio( stream, filename ).read_string();  // Closes stream unless c'tor throws while copying filename -- not a problem here.
         std::cerr << "SIZE " << content.size() << std::endl;
         // TAO_PEGTL_TEST_ASSERT( content.size() == 154 );
      }
      {
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         array_cstream_input<> in( stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
      {
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         alloc_cstream_input<> in( 500, 10, stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
      {
         std::array< char, 500 > buffer;
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         other_cstream_input<> in( buffer.data(), 500, 10, stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
      {
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         array_cstream_input< tao_stream_eol, std::string, std::string > in( "filename", stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
      {
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         alloc_cstream_input< tao_stream_eol, const char*, const char* > in( "filename", 500, 10, stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
      {
         std::array< char, 500 > buffer;
         std::FILE* stream = internal::read_file_open( filename );
         TAO_PEGTL_TEST_ASSERT( stream != nullptr );
         other_cstream_input< tao_stream_eol, const char*, const char* > in( "filename", buffer.data(), 500, 10, stream );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         std::fclose( stream );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
