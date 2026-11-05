// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>
#include <span>

#include "test.hpp"

#include <tao/pegtl/stream.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Data >
   using func_input = internal::input_with_funcs< internal::view_input< Data > >;

   template< typename Input, typename T >
   concept can_peek_as = requires( const Input& in ) {
      in.template peek_as< T >();
   };

   template< typename Input, bool Expected >
   constexpr bool verify_byte_peeks()
   {
      static_assert( requires( const Input& in ) { in.peek_char(); } == Expected );
      static_assert( requires( const Input& in ) { in.peek_byte(); } == Expected );
      static_assert( requires( const Input& in ) { in.peek_int8(); } == Expected );
      static_assert( requires( const Input& in ) { in.peek_uint8(); } == Expected );
      return true;
   }

   template< typename Input, bool String, bool Vector >
   constexpr bool verify_materializations()
   {
      static_assert( requires( const Input& in ) { in.string(); } == String );
      static_assert( requires( const Input& in ) { in.string_view(); } == String );
      static_assert( requires( const Input& in ) { in.vector(); } == Vector );
      return true;
   }

   struct token
   {
      char value;
   };

   static_assert( can_peek_as< func_input< char >, char > );
   static_assert( can_peek_as< func_input< char >, const char& > );
   static_assert( !can_peek_as< func_input< char >, char16_t > );
   static_assert( !can_peek_as< func_input< char >, void > );
   static_assert( can_peek_as< func_input< char32_t >, char32_t > );
   static_assert( can_peek_as< func_input< token >, token > );
   static_assert( !can_peek_as< func_input< token >, char > );

   static_assert( verify_byte_peeks< func_input< char >, true >() );
   static_assert( verify_byte_peeks< func_input< std::byte >, true >() );
   static_assert( verify_byte_peeks< func_input< char32_t >, false >() );
   static_assert( verify_byte_peeks< func_input< token >, false >() );

   static_assert( verify_materializations< func_input< char >, true, true >() );
   static_assert( verify_materializations< func_input< const char >, true, false >() );
   static_assert( verify_materializations< func_input< volatile char >, false, false >() );
   static_assert( verify_materializations< func_input< std::byte >, false, true >() );
   static_assert( verify_materializations< func_input< unsigned char >, false, true >() );
   static_assert( verify_materializations< func_input< char32_t >, false, true >() );
   static_assert( verify_materializations< func_input< token >, false, true >() );
   static_assert( verify_materializations< func_input< std::unique_ptr< int > >, false, false >() );

   template< typename Input >
   concept has_span = requires( const Input& in ) { in.span(); };

   // Streams require a mutable size query; action inputs expose a fixed, const range.
   static_assert( verify_materializations< array_cstring_input<>, false, false >() );
   static_assert( verify_materializations< action_input< array_cstring_input<> >, true, true >() );
   static_assert( !has_span< array_cstring_input<> > );
   static_assert( has_span< action_input< array_cstring_input<> > > );

   template< typename Input, typename Position >
   concept has_line_view = requires( Input& in, const Position& pos ) {
      { in.line_view_at( pos ) } -> std::same_as< std::string_view >;
   };

   struct mutable_lines
      : internal::view_input< char >
   {
      using view_input::view_input;

      const char* begin_of_line( const int& )
      {
         return current();
      }

      const char* end_of_line_or_file( const int& )
      {
         return end();
      }
   };

   using text_input = text_view_input< scan::lf >;
   static_assert( has_line_view< text_input, text_input::error_position_t > );
   static_assert( !has_line_view< text_input, int > );
   static_assert( !has_line_view< func_input< char >, pointer_position< char > > );
   static_assert( !has_line_view< action_input< text_input >, text_input::error_position_t > );
   static_assert( has_line_view< internal::input_with_funcs< mutable_lines >, int > );

   void unit_test()
   {
      {
         const func_input< char > in( "abc" );
         TAO_PEGTL_TEST_ASSERT( in.peek_as< char >( 1 ) == 'b' );
         TAO_PEGTL_TEST_ASSERT( in.peek_char() == 'a' );
         TAO_PEGTL_TEST_ASSERT( in.peek_byte( 1 ) == std::byte( 'b' ) );
         TAO_PEGTL_TEST_ASSERT( in.peek_int8( 2 ) == 'c' );
         TAO_PEGTL_TEST_ASSERT( in.peek_uint8( 2 ) == 'c' );
         TAO_PEGTL_TEST_ASSERT( in.string() == "abc" );
         TAO_PEGTL_TEST_ASSERT( in.string_view() == "abc" );
         TAO_PEGTL_TEST_ASSERT( in.vector() == std::vector< char >( { 'a', 'b', 'c' } ) );
         const auto view = in.span();
         static_assert( std::same_as< decltype( in.span() ), std::span< const char > > );
         TAO_PEGTL_TEST_ASSERT( view.data() == in.current() && view.size() == 3 );
      }
      {
         const std::byte data[] = { std::byte( 'a' ), std::byte( 'b' ) };
         const func_input< std::byte > in( data, 2 );
         TAO_PEGTL_TEST_ASSERT( in.peek_char( 1 ) == 'b' );
         TAO_PEGTL_TEST_ASSERT( in.peek_byte() == data[ 0 ] );
         TAO_PEGTL_TEST_ASSERT( in.vector() == std::vector< std::byte >( data, data + 2 ) );
         TAO_PEGTL_TEST_ASSERT( in.span().data() == data && in.span().size() == 2 );
      }
      {
         const char32_t data[] = { U'a', U'b' };
         const func_input< char32_t > in( data, 2 );
         TAO_PEGTL_TEST_ASSERT( in.peek_as< char32_t >( 1 ) == U'b' );
         TAO_PEGTL_TEST_ASSERT( in.vector() == std::vector< char32_t >( data, data + 2 ) );
      }
      {
         text_input in( "abc\ndef" );
         in.consume< eol_exclude_tag >( 1 );
         TAO_PEGTL_TEST_ASSERT( in.line_view_at( in.current_position() ) == "abc" );
      }
      {
         internal::input_with_funcs< mutable_lines > in( "abc" );
         TAO_PEGTL_TEST_ASSERT( in.line_view_at( 0 ) == "abc" );
      }
      {
         array_cstring_input<> in( "abc" );
         in.require( 3 );
         const auto begin = in.rewind_position();
         in.consume< eol_exclude_tag >( 2 );
         const action_input< array_cstring_input<> > ai( begin, in );
         TAO_PEGTL_TEST_ASSERT( ai.string() == "ab" );
         TAO_PEGTL_TEST_ASSERT( ai.string_view() == "ab" );
         TAO_PEGTL_TEST_ASSERT( ai.vector() == std::vector< char >( { 'a', 'b' } ) );
         TAO_PEGTL_TEST_ASSERT( ai.span().data() == ai.begin() && ai.span().size() == 2 );
      }
      {
         const func_input< char > empty( "" );
         TAO_PEGTL_TEST_ASSERT( empty.span().empty() );
         std::unique_ptr< int > data[] = { std::make_unique< int >( 42 ), std::make_unique< int >( 43 ) };
         func_input< std::unique_ptr< int > > in( data, 2 );
         const auto view = in.span();
         TAO_PEGTL_TEST_ASSERT( view.data() == data && *view.front() == 42 );
         *data[ 0 ] = 44;
         TAO_PEGTL_TEST_ASSERT( *view.front() == 44 );
         in.consume< eol_exclude_tag >( 1 );
         TAO_PEGTL_TEST_ASSERT( in.span().data() == data + 1 && in.span().size() == 1 );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
