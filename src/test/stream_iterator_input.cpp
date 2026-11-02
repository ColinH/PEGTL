// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <cerrno>
#include <concepts>
#include <fstream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "test.hpp"

#include <tao/pegtl/stream.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct file_content
      : seq< string< "dummy content" >, one< '\n' >, discard >
   {};

   struct file_grammar
      : seq< rep_min_max< 11, 11, file_content >, eof >
   {};

   template< typename Iterator, typename Sentinel = Iterator >
   concept has_iterator_reader = requires { typename internal::iterator_reader< Iterator, Sentinel >; };

   using counted_iterator = std::counted_iterator< const char* >;
   using sentinel = std::default_sentinel_t;
   using move_iterator = std::ranges::iterator_t< std::ranges::istream_view< char > >;

   static_assert( has_iterator_reader< const char* > );
   static_assert( has_iterator_reader< counted_iterator, sentinel > );
   static_assert( !has_iterator_reader< int > );
   static_assert( !has_iterator_reader< const char*, int > );
   static_assert( !has_iterator_reader< const std::string* > );

   // This standard range exercises move-only iterators with void postfix increment.
   static_assert( std::input_iterator< move_iterator > );
   static_assert( !std::copy_constructible< move_iterator > );
   static_assert( std::same_as< decltype( std::declval< move_iterator& >()++ ), void > );
   static_assert( std::constructible_from< internal::iterator_reader< move_iterator, sentinel >, move_iterator, sentinel > );
   static_assert( !std::constructible_from< internal::iterator_reader< move_iterator, sentinel >, move_iterator&, sentinel > );

   constexpr char range_data[] = "a\nb\nc\n";

   struct range_grammar
      : seq< rep< 3, alpha, eol >, eof >
   {};

   template< typename Input, typename... Args >
   void verify_range( Args&&... args )
   {
      Input in( "range", std::forward< Args >( args )... );
      TAO_PEGTL_TEST_ASSERT( in.buffer_capacity() == 16 );
      TAO_PEGTL_TEST_ASSERT( in.buffer_chunk_size() == 2 );
      TAO_PEGTL_TEST_ASSERT( parse< range_grammar >( in ) );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
      TAO_PEGTL_TEST_ASSERT( in.direct_count() == sizeof( range_data ) - 1 );
      TAO_PEGTL_TEST_ASSERT( in.direct_source() == "range" );
      if constexpr( requires { in.direct_line(); } ) {
         TAO_PEGTL_TEST_ASSERT( in.direct_line() == 4 );
         TAO_PEGTL_TEST_ASSERT( in.direct_column() == 1 );
      }
   }

   void test_sentinels()
   {
      const counted_iterator begin( range_data, sizeof( range_data ) - 1 );
      const sentinel end;
      std::array< char, 16 > buffer;

      verify_range< array_iterator_input< counted_iterator, scan::lf, std::string, std::string, 16, 2, sentinel > >( begin, end );
      verify_range< alloc_iterator_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( 16, 2, begin, end );
      verify_range< other_iterator_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( buffer.data(), 16, 2, begin, end );

      verify_range< array_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, 16, 2, sentinel > >( begin, end );
      verify_range< alloc_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( 16, 2, begin, end );
      verify_range< other_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( buffer.data(), 16, 2, begin, end );

      verify_range< array_text_iterator_input< counted_iterator, scan::lf, std::string, std::string, 16, 2, sentinel > >( begin, end );
      verify_range< alloc_text_iterator_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( 16, 2, begin, end );
      verify_range< other_text_iterator_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( buffer.data(), 16, 2, begin, end );

      verify_range< array_text_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, 16, 2, sentinel > >( begin, end );
      verify_range< alloc_text_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( 16, 2, begin, end );
      verify_range< other_text_iterator_auto_input< counted_iterator, scan::lf, std::string, std::string, sentinel > >( buffer.data(), 16, 2, begin, end );
   }

   void test_move_iterators()
   {
      {
         std::istringstream stream( range_data );
         stream >> std::noskipws;
         auto range = std::views::istream< char >( stream );
         verify_range< array_iterator_input< move_iterator, scan::lf, std::string, std::string, 16, 2, sentinel > >( range.begin(), range.end() );
      }
      {
         std::istringstream stream( range_data );
         stream >> std::noskipws;
         auto range = std::views::istream< char >( stream );
         verify_range< alloc_text_iterator_input< move_iterator, scan::lf, std::string, std::string, sentinel > >( 16, 2, range.begin(), range.end() );
      }
      {
         std::array< char, 16 > buffer;
         std::istringstream stream( range_data );
         stream >> std::noskipws;
         auto range = std::views::istream< char >( stream );
         verify_range< other_text_iterator_auto_input< move_iterator, scan::lf, std::string, std::string, sentinel > >( buffer.data(), 16, 2, range.begin(), range.end() );
      }
   }

   void test_reader()
   {
      internal::iterator_reader reader( counted_iterator( range_data, sizeof( range_data ) - 1 ), sentinel() );
      std::array< char, 8 > buffer;
      TAO_PEGTL_TEST_ASSERT( reader.read( buffer.data(), 0 ) == 0 );
      TAO_PEGTL_TEST_ASSERT( reader.read( buffer.data(), 2 ) == 2 );
      TAO_PEGTL_TEST_ASSERT( std::string_view( buffer.data(), 2 ) == "a\n" );
      TAO_PEGTL_TEST_ASSERT( reader.read( buffer.data(), buffer.size() ) == 4 );
      TAO_PEGTL_TEST_ASSERT( std::string_view( buffer.data(), 4 ) == "b\nc\n" );
      TAO_PEGTL_TEST_ASSERT( reader.read( buffer.data(), buffer.size() ) == 0 );
   }

   void unit_test()
   {
      test_sentinels();
      test_move_iterators();
      test_reader();

      const char* filename = "src/test/data/test_data.txt";
      const std::istreambuf_iterator< char > end;
      {
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         array_iterator_input< decltype( begin ) > in( begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         alloc_iterator_input< decltype( begin ) > in( 100, 90, begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         std::array< char, 100 > buffer;
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         other_iterator_input< decltype( begin ) > in( buffer.data(), 100, 90, begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         array_iterator_input< decltype( begin ), std::string, std::string > in( "filename", begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         alloc_iterator_input< decltype( begin ), const char*, const char* > in( "filename", 100, 90, begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         std::array< char, 100 > buffer;
         std::ifstream stream( filename );
         std::istreambuf_iterator< char > begin( stream );
         other_iterator_input< decltype( begin ), const char*, const char* > in( "filename", buffer.data(), 100, 90, begin, end );
         TAO_PEGTL_TEST_ASSERT( parse< file_grammar >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
