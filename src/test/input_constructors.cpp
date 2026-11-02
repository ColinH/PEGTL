// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>
#include <span>
#include <utility>

#include "test.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   using literal = const char ( & )[ 4 ];
   using plain_input = view_input< scan::lf >;
   using sourced_input = view_input< scan::lf, char, std::string >;
   using tracked_input = text_view_input< scan::lf, char, std::string >;

   static_assert( std::constructible_from< plain_input, literal > );
   static_assert( std::constructible_from< plain_input, std::string& > );
   static_assert( std::constructible_from< plain_input, const std::string& > );
   static_assert( std::constructible_from< plain_input, std::string_view > );
   static_assert( !std::constructible_from< plain_input > );
   static_assert( !std::constructible_from< plain_input, int > );
   static_assert( !std::constructible_from< plain_input, std::string > );
   static_assert( !std::constructible_from< plain_input, literal, int, int > );
   static_assert( std::constructible_from< plain_input, std::span< const char > > );
   static_assert( std::constructible_from< plain_input, std::span< const char, 3 > > );
   static_assert( std::constructible_from< base_input<>, std::span< const char > > );

   static_assert( std::constructible_from< sourced_input, std::string&, literal > );
   static_assert( std::constructible_from< sourced_input, const std::string&, literal > );
   static_assert( std::constructible_from< sourced_input, std::string, literal > );
   static_assert( !std::constructible_from< sourced_input, int, literal > );
   static_assert( !std::constructible_from< sourced_input, std::string, int > );
   static_assert( !std::constructible_from< sourced_input, std::string > );
   static_assert( std::constructible_from< tracked_input, std::string, literal > );
   static_assert( !std::constructible_from< tracked_input, int, literal > );
   static_assert( !std::constructible_from< tracked_input, std::string, int > );
   static_assert( std::constructible_from< sourced_input, std::string, std::span< const char > > );
   static_assert( std::constructible_from< tracked_input, std::string, std::span< const char > > );

   // The byte-sized string constructors must reject wide and non-scalar data.
   struct token
   {
      char value;
   };

   static_assert( std::constructible_from< view_input< scan::lf, std::byte >, literal > );
   static_assert( !std::constructible_from< view_input< scan::lf, char32_t >, literal > );
   static_assert( !std::constructible_from< view_input< scan::lf, char32_t >, std::string_view > );
   static_assert( !std::constructible_from< view_input< scan::lf, token >, const std::string& > );
   static_assert( std::constructible_from< view_input< scan::lf, token >, const token*, std::size_t > );

   using offset_input = input_with_offset< plain_input >;
   static_assert( std::constructible_from< offset_input, count_position, literal > );
   static_assert( std::constructible_from< offset_input, const count_position&, literal > );
   static_assert( !std::constructible_from< offset_input, count_position, int > );
   static_assert( !std::constructible_from< offset_input, const count_position&, int > );

   static_assert( std::constructible_from< file_input<>, std::filesystem::path > );
   static_assert( std::constructible_from< file_input<>, const std::filesystem::path& > );
   static_assert( !std::constructible_from< file_input<>, std::filesystem::path, int > );
   static_assert( !std::constructible_from< file_input<>, const std::filesystem::path&, int > );

   using move_source = std::unique_ptr< int >;
   using move_sourced_input = view_input< scan::lf, char, move_source >;
   using move_tracked_input = text_view_input< scan::lf, char, move_source >;
   using position = position_with_source< move_source, count_position >;

   static_assert( std::constructible_from< position, move_source > );
   static_assert( !std::constructible_from< position, const move_source& > );
   static_assert( std::constructible_from< position, move_source, count_position > );
   static_assert( !std::constructible_from< position, move_source&, count_position > );
   static_assert( !std::constructible_from< position, move_source, std::string > );
   static_assert( std::constructible_from< move_sourced_input, move_source, literal > );
   static_assert( !std::constructible_from< move_sourced_input, move_source&, literal > );
   static_assert( std::constructible_from< move_tracked_input, move_source, literal > );
   static_assert( !std::constructible_from< move_tracked_input, move_source&, literal > );

   struct permissive_source
   {
      template< typename T >
      permissive_source( T&& )
      {}
   };

   struct default_input
      : internal::view_input< char >
   {
      default_input()
         : view_input( "abc" )
      {}
   };

   void unit_test()
   {
      {
         const char data[] = "abc";
         const std::span< const char > dynamic( data, 3 );
         const std::span< const char, 3 > fixed( data, 3 );
         plain_input plain( fixed );
         TAO_PEGTL_TEST_ASSERT( plain.current() == data && plain.size() == 3 );
         sourced_input sourced( "source", dynamic );
         TAO_PEGTL_TEST_ASSERT( sourced.direct_source() == "source" );
         TAO_PEGTL_TEST_ASSERT( sourced.current() == data && sourced.size() == 3 );
         tracked_input tracked( "source", dynamic );
         TAO_PEGTL_TEST_ASSERT( tracked.current() == data && tracked.size() == 3 );
         base_input<> base( dynamic );
         TAO_PEGTL_TEST_ASSERT( base.current() == data && base.size() == 3 );
         plain_input empty( dynamic.first( 0 ) );
         TAO_PEGTL_TEST_ASSERT( empty.empty() );
      }
      {
         // Copying a mutable input must preserve its start as well as its cursor.
         using started_input = internal::input_with_start< internal::view_input< char > >;
         started_input original( "abc" );
         original.consume< eol_exclude_tag >( 1 );
         const started_input copy( original );
         TAO_PEGTL_TEST_ASSERT( copy.start() == original.start() );
         TAO_PEGTL_TEST_ASSERT( copy.current() == original.current() );
         TAO_PEGTL_TEST_ASSERT( copy.current_position().count == 1 );
         const started_input moved( std::move( original ) );
         TAO_PEGTL_TEST_ASSERT( moved.start() == copy.start() );
         TAO_PEGTL_TEST_ASSERT( moved.current() == copy.current() );
      }
      {
         // A source that accepts any argument must not intercept input copies.
         using input = internal::input_with_source< permissive_source, permissive_source, default_input >;
         input original( 0 );
         original.consume< eol_exclude_tag >( 1 );
         const input copy( original );
         TAO_PEGTL_TEST_ASSERT( copy.current() == original.current() );
      }
      {
         auto s = std::make_unique< int >( 42 );
         const auto* const p = s.get();
         const move_sourced_input in( std::move( s ), "abc" );
         TAO_PEGTL_TEST_ASSERT( in.direct_source().get() == p );
         TAO_PEGTL_TEST_ASSERT( in.string_view() == "abc" );
      }
      {
         const move_tracked_input in( std::make_unique< int >( 42 ), "abc" );
         TAO_PEGTL_TEST_ASSERT( *in.direct_source() == 42 );
         TAO_PEGTL_TEST_ASSERT( in.direct_position().count == 0 );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
