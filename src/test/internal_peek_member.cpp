// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "test.hpp"

#include <tao/pegtl/internal/peek_member.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   static int the_integer = -2;

   struct getter_error
   {};

   struct token
   {
      explicit token( std::string&& s ) noexcept
         : string( std::move( s ) )
      {}

      std::string string;

      int local1 = 12;
      const int local2 = 23;

      int* global1 = &the_integer;
      const int* global2 = &the_integer;
      int* const global3 = &the_integer;
      const int* const global4 = &the_integer;

      [[nodiscard]] int method1() const noexcept  // NOLINT(readability-convert-member-functions-to-static)
      {
         return 503;
      }

      const int method2_int = 102;

      [[nodiscard]] const int& method2() const noexcept
      {
         return method2_int;
      }

      const int method3_int = 3333;

      [[nodiscard]] const int* method3() const noexcept
      {
         return &method3_int;
      }

      int other1 = 901;
      int other2 = 902;
      int other3 = 903;

      mutable unsigned calls = 0;

      [[nodiscard]] int counted() const
      {
         ++calls;
#if defined( __cpp_exceptions )
         if( other1 < 0 ) {
            throw getter_error();
         }
#endif
         return other1;
      }

      [[nodiscard]] const int& lvalue() const& noexcept
      {
         return local1;
      }

      [[nodiscard]] int* pointer() const noexcept
      {
         return global1;
      }

      [[nodiscard]] int&& rvalue() const noexcept
      {
         return std::move( *global1 );
      }
   };

   [[nodiscard]] int get_other1( const token& t ) noexcept
   {
      return t.other1;
   }

   [[nodiscard]] const int& get_other2( const token& t ) noexcept
   {
      return t.other2;
   }

   [[nodiscard]] const int* get_other3( const token& t ) noexcept
   {
      return &t.other3;
   }

   [[nodiscard]] int* get_pointer( const token& t ) noexcept
   {
      return t.global1;
   }

   [[nodiscard]] int get_counted( const token& t )
   {
      return t.counted();
   }

   using tokens = std::vector< token >;

   using test_input = internal::input_with_fakes< internal::copy_input< tokens > >;

   static int test_integer = -1;

   [[nodiscard]] bool func1( const int i ) noexcept
   {
      test_integer = i;
      return true;
   }

   using view = view_input< void, token >;

   struct copied_value
   {
      int value = 123;

      copied_value() = default;
      copied_value( const copied_value& ) = default;
      copied_value( copied_value&& ) = delete;
   };

   [[nodiscard]] copied_value get_copy( const token& ) noexcept
   {
      return copied_value();
   }

   struct throwing_size_input
      : view
   {
      [[nodiscard]] std::size_t size( std::size_t = 0 ) const noexcept( false )
      {
         return view::size();
      }
   };

   struct throwing_current_input
      : view
   {
      [[nodiscard]] const token* current( const std::size_t offset = 0 ) const noexcept( false )
      {
         return view::current( offset );
      }
   };

   static_assert( std::is_same_v< internal::peek_member< &token::global4 >::data_t, int > );
   static_assert( std::is_same_v< internal::peek_member< &token::local1 >::pair_t, internal::data_and_size< int, void > > );
   static_assert( std::is_same_v< internal::peek_member< &get_other1 >::pair_t, internal::data_and_size< int, std::uint8_t > > );
   static_assert( std::is_same_v< internal::peek_member< &token::lvalue >::pair_t, internal::data_and_size< int, void > > );
   static_assert( std::is_same_v< internal::peek_member< &get_pointer >::pair_t, internal::data_and_size< int, void > > );
   static_assert( noexcept( internal::peek_member< &token::local1 >::peek( std::declval< view& >() ) ) );
   static_assert( noexcept( internal::peek_member< &get_other1 >::peek( std::declval< view& >() ) ) );
   static_assert( noexcept( internal::peek_member< &token::method2 >::peek( std::declval< view& >() ) ) );
   static_assert( noexcept( internal::peek_member< &get_pointer >::peek( std::declval< view& >() ) ) );
   static_assert( !noexcept( internal::peek_member< &token::counted >::peek( std::declval< view& >() ) ) );
   static_assert( !noexcept( internal::peek_member< &get_counted >::peek( std::declval< view& >() ) ) );
   static_assert( !noexcept( internal::peek_member< &token::local1 >::peek( std::declval< throwing_size_input& >() ) ) );
   static_assert( !noexcept( internal::peek_member< &token::local1 >::peek( std::declval< throwing_current_input& >() ) ) );

   void test_results( tokens& data )
   {
      view in( data );
      const auto value = internal::peek_member< &get_other1 >::peek( in, 1 );
      const auto reference = internal::peek_member< &token::lvalue >::peek( in, 1 );
      TAO_PEGTL_TEST_ASSERT( value && value.size() == 1 && value.data() == 901 );
      TAO_PEGTL_TEST_ASSERT( reference.pointer() == std::addressof( data[ 1 ].local1 ) );
      data[ 1 ].other1 = 42;
      data[ 1 ].local1 = 43;
      TAO_PEGTL_TEST_ASSERT( value.data() == 901 );
      TAO_PEGTL_TEST_ASSERT( reference.data() == 43 );

      const auto copy = internal::peek_member< &get_copy >::peek( in );
      TAO_PEGTL_TEST_ASSERT( copy && copy.data().value == 123 );

      const auto counted = internal::peek_member< &token::counted >::peek( in, 1 );
      TAO_PEGTL_TEST_ASSERT( counted && counted.data() == 42 );
      TAO_PEGTL_TEST_ASSERT( data[ 0 ].calls == 0 && data[ 1 ].calls == 1 );
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &token::counted >::peek( in, data.size() ) );
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &get_counted >::peek( in, data.size() ) );
      TAO_PEGTL_TEST_ASSERT( data[ 1 ].calls == 1 );
      TAO_PEGTL_TEST_ASSERT( in.size() == data.size() );

      const auto pointer = internal::peek_member< &token::pointer >::peek( in );
      TAO_PEGTL_TEST_ASSERT( pointer.pointer() == data[ 0 ].global1 );
      const auto rvalue = internal::peek_member< &token::rvalue >::peek( in );
      TAO_PEGTL_TEST_ASSERT( rvalue.pointer() == data[ 0 ].global1 );
      data[ 0 ].global1 = nullptr;
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &token::global1 >::peek( in ) );
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &token::pointer >::peek( in ) );
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &get_pointer >::peek( in ) );
      test_integer = -1;
      TAO_PEGTL_TEST_ASSERT( !parse< function< func1, internal::peek_member< &get_pointer > > >( in ) );
      TAO_PEGTL_TEST_ASSERT( test_integer == -1 && in.size() == data.size() );

#if defined( __cpp_exceptions )
      data[ 0 ].other1 = -1;
      view empty( data.data(), std::size_t( 0 ) );
      TAO_PEGTL_TEST_ASSERT( !internal::peek_member< &get_counted >::peek( empty ) );
      TAO_PEGTL_TEST_ASSERT( data[ 0 ].calls == 0 );
      try {
         static_cast< void >( internal::peek_member< &get_counted >::peek( in ) );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const getter_error& ) {
         TAO_PEGTL_TEST_ASSERT( data[ 0 ].calls == 1 );
         TAO_PEGTL_TEST_ASSERT( in.size() == data.size() );
      }
#endif
   }

   void unit_test()
   {
      tokens te;
      tokens ts;
      ts.emplace_back( "0" );
      ts.emplace_back( "token" );
      ts.emplace_back( "something" );
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::local1 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 12 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::local2 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 23 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         the_integer = 42;
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::global1 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 42 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         the_integer = -120;
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::global2 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == -120 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         the_integer = 0;
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::global3 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 0 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         the_integer = 65536;
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::global4 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 65536 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::method1 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 503 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::method2 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 102 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &token::method3 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 3333 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &get_other1 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 901 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &get_other2 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 902 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      {
         test_integer = -1;
         test_input in( ts );
         using peek_type = internal::peek_member< &get_other3 >;
         static_assert( peek_type::bulk< test_input >() );
         static_assert( peek_type::size< test_input >() == 1 );
         const auto b = parse< function< func1, peek_type > >( in );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( test_integer == 903 );
         test_input ie( te );
         TAO_PEGTL_TEST_ASSERT( !parse< function< func1, peek_type > >( ie ) );
      }
      test_results( ts );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
