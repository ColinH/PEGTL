// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>
#include <utility>

#include "test.hpp"

#include <tao/pegtl/stream.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   static_assert( std::constructible_from< array_cstring_buffer<>, const char* > );
   static_assert( !std::constructible_from< array_cstring_buffer<> > );
   static_assert( !std::constructible_from< array_cstring_buffer<>, int > );
   static_assert( std::constructible_from< alloc_cstring_buffer, std::size_t, std::size_t, const char* > );
   static_assert( !std::constructible_from< alloc_cstring_buffer, std::size_t, std::size_t, int > );
   static_assert( std::constructible_from< other_cstring_buffer, char*, std::size_t, std::size_t, const char* > );
   static_assert( !std::constructible_from< other_cstring_buffer, char*, std::size_t, std::size_t, int > );

   static_assert( std::constructible_from< array_istream_input<>, std::istream& > );
   static_assert( !std::constructible_from< array_istream_input<>, std::istream > );
   static_assert( std::constructible_from< array_cstring_input<>, const char* > );
   static_assert( !std::constructible_from< array_cstring_input<>, int > );
   static_assert( !std::constructible_from< array_cstring_input<> > );

   using sourced_input = array_cstring_input< void, std::string >;
   using tracked_input = text_stream_input< array_cstring_buffer<>, scan::lf, std::string >;
   static_assert( std::constructible_from< sourced_input, std::string, const char* > );
   static_assert( !std::constructible_from< sourced_input, int, const char* > );
   static_assert( !std::constructible_from< sourced_input, std::string, int > );
   static_assert( std::constructible_from< tracked_input, std::string, const char* > );
   static_assert( !std::constructible_from< tracked_input, int, const char* > );
   static_assert( !std::constructible_from< tracked_input, std::string, int > );

   struct move_reader
   {
      explicit move_reader( std::unique_ptr< int >&& value )
         : state( std::move( value ) )
      {}

      std::size_t read( char*, std::size_t )
      {
         return 0;
      }

      std::unique_ptr< int > state;
   };

   using move_source = std::unique_ptr< int >;
   using buffer = internal::array_buffer< char, move_reader, 16, 4 >;
   using move_sourced_input = stream_input< buffer, void, move_source >;
   using move_tracked_input = text_stream_input< buffer, scan::lf, move_source >;
   static_assert( std::constructible_from< buffer, move_source > );
   static_assert( !std::constructible_from< buffer, move_source& > );
   static_assert( std::constructible_from< move_sourced_input, move_source, move_source > );
   static_assert( !std::constructible_from< move_sourced_input, move_source&, move_source > );
   static_assert( !std::constructible_from< move_sourced_input, move_source, move_source& > );
   static_assert( std::constructible_from< move_tracked_input, move_source, move_source > );
   static_assert( !std::constructible_from< move_tracked_input, move_source&, move_source > );
   static_assert( !std::constructible_from< move_tracked_input, move_source, move_source& > );

   struct permissive_reader
   {
      template< typename T >
      explicit permissive_reader( T&& )
      {}
   };

   // Even a permissive reader must not bypass a buffer's deleted copy/move operations.
   using noncopyable_buffer = internal::array_buffer< char, permissive_reader, 16, 4 >;
   static_assert( !std::constructible_from< noncopyable_buffer, noncopyable_buffer& > );
   static_assert( !std::constructible_from< noncopyable_buffer, const noncopyable_buffer& > );
   static_assert( !std::constructible_from< noncopyable_buffer, noncopyable_buffer > );

   void unit_test()
   {
      {
         move_sourced_input in( std::make_unique< int >( 42 ), std::make_unique< int >( 17 ) );
         TAO_PEGTL_TEST_ASSERT( *in.direct_source() == 42 );
         TAO_PEGTL_TEST_ASSERT( *in.direct_reader().state == 17 );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         move_tracked_input in( std::make_unique< int >( 42 ), std::make_unique< int >( 17 ) );
         TAO_PEGTL_TEST_ASSERT( *in.direct_source() == 42 );
         TAO_PEGTL_TEST_ASSERT( *in.direct_reader().state == 17 );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
