// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <vector>

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"

#include <tao/pegtl/member.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct token
   {
      std::string value;
      int dummy;
   };

   using inner_input = internal::input_with_funcs< internal::input_with_fakes< internal::input_with_start< internal::view_input< char > > > >;
   using outer_input = internal::input_with_fakes< internal::copy_input< std::vector< token > > >;

   using inner_grammar = seq< string< "foo" >, eof >;
   using member_grammar = seq< member::nested< &token::value, inner_grammar >, eof >;

   struct peek_string
   {
      using data_t = std::string;

      template< typename ParseInput >
      [[nodiscard]] static constexpr bool bulk() noexcept
      {
         return true;
      }

      template< typename ParseInput >
      [[nodiscard]] static constexpr std::size_t size() noexcept
      {
         return 1;
      }

      template< typename ParseInput >
      [[nodiscard]] static auto peek( ParseInput& in, const std::size_t offset = 0 ) noexcept( noexcept( in.size( 42 ) ) )
      {
         return internal::data_and_size( ( in.size( offset + 1 ) >= ( offset + 1 ) ) ? in.current( offset ) : nullptr );
      }
   };

   using nested_grammar = seq< nested< inner_grammar, peek_string >, eof >;

   void test_inner()
   {
      inner_input in( "foo" );
      TAO_PEGTL_TEST_ASSERT( in.size() == 3 );
      const auto b = parse< inner_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( b );
   }

   void test_success()
   {
      outer_input in( {
         { "foo", 1 }
      } );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
      const auto b = parse< member_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( b );
   }

   void test_failure1()
   {
      outer_input in( {
         { "bar", 1 }
      } );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
      const auto b = parse< member_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
   }

   void test_failure2()
   {
      outer_input in( {} );
      TAO_PEGTL_TEST_ASSERT( in.size() == 0 );
      const auto b = parse< member_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
   }

   void test_failure3()
   {
      outer_input in( {
         { "foo", 1 },
         { "foo", 1 }
      } );
      TAO_PEGTL_TEST_ASSERT( in.size() == 2 );
      const auto b = parse< member_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
   }

   void test_nested_success()
   {
      const std::vector< std::string > data = { "foo" };
      view_input< void, std::string > in( data );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
      const auto b = parse< nested_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( in.size() == 0 );
   }

   void test_nested_failure1()
   {
      const std::vector< std::string > data = { "bar" };
      view_input< void, std::string > in( data );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
      const auto b = parse< nested_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
   }

   void test_nested_failure2()
   {
      const std::vector< std::string > data;
      view_input< void, std::string > in( data );
      TAO_PEGTL_TEST_ASSERT( in.size() == 0 );
      const auto b = parse< nested_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
      TAO_PEGTL_TEST_ASSERT( in.size() == 0 );
   }

   void test_nested_failure3()
   {
      const std::vector< std::string > data = { "foo", "foo" };
      view_input< void, std::string > in( data );
      TAO_PEGTL_TEST_ASSERT( in.size() == 2 );
      const auto b = parse< nested_grammar >( in );
      TAO_PEGTL_TEST_ASSERT( !b );
      TAO_PEGTL_TEST_ASSERT( in.size() == 1 );
   }

   void unit_test()
   {
      verify_meta< nested< inner_grammar, peek_string >, internal::nested< peek_string, inner_grammar > >();

      const std::vector< std::string > ctrl_data = { "foo" };
      view_input< void, std::string > enabled_in( ctrl_data );
      verify_ctrl_enabled< nested< inner_grammar, peek_string > >( __LINE__, __FILE__, enabled_in );
      view_input< void, std::string > disabled_in( ctrl_data );
      verify_ctrl_disabled< internal::nested< peek_string, inner_grammar > >( __LINE__, __FILE__, disabled_in );

      test_inner();
      test_success();
      test_failure1();
      test_failure2();
      test_failure3();
      test_nested_success();
      test_nested_failure1();
      test_nested_failure2();
      test_nested_failure3();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
