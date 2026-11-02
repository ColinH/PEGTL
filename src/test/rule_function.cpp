// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <memory>
#include <type_traits>
#include <utility>

#include "test.hpp"
#include "test_utility.hpp"
#include "verify_ctrl.hpp"

#include "peek_data.hpp"

#include <tao/pegtl/eol_unknown_tag.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   static bool called = false;

   template< auto F, typename P = void >
   void unit_test_1()
   {
      int i = 42;
      char c = 'a';
      double d = 42.0;
      called = false;
      text_view_input< scan::lf > in( "f" );
      TAO_PEGTL_TEST_ASSERT( parse< seq< function< F, P >, eof > >( in, i, &c, d ) );
      TAO_PEGTL_TEST_ASSERT( called );
   }

   [[nodiscard]] bool func0( text_view_input< scan::lf >& in )
   {
      called = true;
      in.consume< eol_unknown_tag >( 1 );
      return true;
   }

   [[nodiscard]] bool func0n( text_view_input< scan::lf >& in ) noexcept
   {
      called = true;
      in.consume< eol_unknown_tag >( 1 );
      return true;
   }

   [[nodiscard]] bool func1( text_view_input< scan::lf >& in, int /*unused*/, char*& /*unused*/, const double& /*unused*/ )
   {
      called = true;
      in.consume< eol_unknown_tag >( 1 );
      return true;
   }

   [[nodiscard]] bool func1n( text_view_input< scan::lf >& in, int /*unused*/, char*& /*unused*/, const double& /*unused*/ ) noexcept
   {
      called = true;
      in.consume< eol_unknown_tag >( 1 );
      return true;
   }

   [[nodiscard]] bool func2( const char /*unused*/, int /*unused*/, char*& /*unused*/, const double& /*unused*/ )
   {
      called = true;
      return true;
   }

   [[nodiscard]] bool func2n( const char /*unused*/, int /*unused*/, char*& /*unused*/, const double& /*unused*/ ) noexcept
   {
      called = true;
      return true;
   }

   [[nodiscard]] bool func3( const char /*unused*/ )
   {
      called = true;
      return true;
   }

   [[nodiscard]] bool func3n( const char /*unused*/ ) noexcept
   {
      called = true;
      return true;
   }

   struct callable
   {
      int value;

      template< typename Input >
      bool operator()( Input& in ) const noexcept
      {
         called = true;
         in.template consume< eol_unknown_tag >( 1 );
         return value == 42;
      }
   };

   struct overloaded_callable
   {
      bool operator()( text_view_input< scan::lf >& in ) const noexcept
      {
         return func0n( in );
      }

      bool operator()( text_view_input< scan::lf >& in, int& count ) const noexcept
      {
         ++count;
         return func0n( in );
      }
   };

   struct throwing_peek
      : internal::peek_char
   {
      template< typename Input >
      static auto peek( Input& in ) noexcept( false )
      {
         return internal::peek_char::peek( in );
      }
   };

   struct throwing_consume_input
      : view_input<>
   {
      using view_input<>::view_input;

      template< typename Rule >
      void consume( const std::size_t count ) noexcept( false )
      {
         view_input<>::template consume< Rule >( count );
      }
   };

   struct throwing_conversion
   {
      operator bool() const noexcept( false )
      {
         return true;
      }
   };

   template< typename Rule, typename Input, typename... States >
   constexpr bool match_is_nothrow = noexcept( Rule::template match< apply_mode::enabled, rewind_mode::required, nothing, normal >( std::declval< Input& >(), std::declval< States& >()... ) );

   static_assert( !match_is_nothrow< function< func0 >, text_view_input< scan::lf > > );
   static_assert( match_is_nothrow< function< func0n >, text_view_input< scan::lf > > );
   static_assert( match_is_nothrow< function< func1n >, text_view_input< scan::lf >, int, char*, const double > );
   static_assert( match_is_nothrow< function< func3n, internal::peek_char >, view_input<> > );
   static_assert( !match_is_nothrow< function< func3, internal::peek_char >, view_input<> > );
   static_assert( !match_is_nothrow< function< func3n, throwing_peek >, view_input<> > );
   static_assert( !match_is_nothrow< function< func3n, internal::peek_char >, throwing_consume_input > );
   static_assert( !match_is_nothrow< function< []( auto& ) noexcept { return throwing_conversion(); } >, view_input<> > );

   void test_callables()
   {
      unit_test_1< []( auto& in ) {
         called = true;
         in.template consume< eol_unknown_tag >( 1 );
         return true;
      } >();
      unit_test_1< []( auto& in, int i, char*& c, const double& d ) noexcept {
         called = true;
         in.template consume< eol_unknown_tag >( 1 );
         return i == 42 && *c == 'a' && d == 42.0;
      } >();
      unit_test_1< []( const auto c ) noexcept {
         called = true;
         return c == 'f';
      },
                   internal::peek_char >();
      unit_test_1< []( const auto& c, int i, char*& state, const double& d ) {
         called = true;
         return c == 'f' && i == 42 && *state == 'a' && d == 42.0;
      },
                   test::peek_data >();
      unit_test_1< callable{ 42 } >();

      int count = 0;
      text_view_input< scan::lf > in( "f" );
      TAO_PEGTL_TEST_ASSERT( parse< function< overloaded_callable{} > >( in, count ) );
      TAO_PEGTL_TEST_ASSERT( count == 1 );
      TAO_PEGTL_TEST_ASSERT( in.empty() );

      constexpr auto predicate = []( const auto c, auto& state ) noexcept {
         ++*state;
         return c == 'x';
      };
      auto state = std::make_unique< int >( 0 );
      view_input empty( "" );
      TAO_PEGTL_TEST_ASSERT( !parse< function< predicate, internal::peek_char > >( empty, state ) );
      TAO_PEGTL_TEST_ASSERT( *state == 0 );
      view_input rejected( "y" );
      TAO_PEGTL_TEST_ASSERT( !parse< function< predicate, internal::peek_char > >( rejected, state ) );
      TAO_PEGTL_TEST_ASSERT( *state == 1 && rejected.size() == 1 );
      view_input accepted( "x" );
      TAO_PEGTL_TEST_ASSERT( parse< function< predicate, internal::peek_char > >( accepted, state ) );
      TAO_PEGTL_TEST_ASSERT( *state == 2 && accepted.empty() );

      TAO_PEGTL_TEST_ASSERT( parse< function< &view_input<>::empty > >( empty ) );
      TAO_PEGTL_TEST_ASSERT( !parse< function< &view_input<>::empty > >( rejected ) );

      struct token
      {
         bool value;

         bool test() const noexcept
         {
            return value;
         }
      };
      const token data[] = { { false }, { true } };
      view_input< void, token > tokens( data, 2 );
      TAO_PEGTL_TEST_ASSERT( !parse< function< &token::value, test::peek_data > >( tokens ) );
      TAO_PEGTL_TEST_ASSERT( tokens.size() == 2 );
      tokens.consume< eol_unknown_tag >( 1 );
      TAO_PEGTL_TEST_ASSERT( parse< function< &token::test, test::peek_data > >( tokens ) );
      TAO_PEGTL_TEST_ASSERT( tokens.empty() );

#if defined( __cpp_exceptions )
      try {
         parse< function< []( const char ) -> bool { throw 42; }, internal::peek_char > >( rejected );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const int value ) {
         TAO_PEGTL_TEST_ASSERT( value == 42 );
         TAO_PEGTL_TEST_ASSERT( rejected.size() == 1 );
      }
#endif
   }

   template< auto F >
   void unit_test_2()
   {
      int i = 42;
      char c = 'a';
      double d = 42.0;
      called = false;
      const char data[] = { char( 0x80 ), 0x00 };
      text_view_input< scan::lf > in( data, 1 );
      TAO_PEGTL_TEST_ASSERT( !parse< seq< function< F, internal::peek_seven >, eof > >( in, i, &c, d ) );
      TAO_PEGTL_TEST_ASSERT( !called );
   }

   void unit_test()
   {
      verify_ctrl_enabled< function< func0 > >( __LINE__, __FILE__, "f" );
      verify_ctrl_disabled< internal::function< void, func0 > >( __LINE__, __FILE__, "f" );

      unit_test_1< func0 >();
      unit_test_1< func1 >();
      unit_test_1< func2, internal::peek_char >();
      unit_test_1< func2, test::peek_data >();
      unit_test_1< func3, internal::peek_char >();
      unit_test_1< func3, test::peek_data >();
      unit_test_1< func0n >();
      unit_test_1< func1n >();
      unit_test_1< func2n, internal::peek_char >();
      unit_test_1< func2n, test::peek_data >();
      unit_test_1< func3n, internal::peek_char >();
      unit_test_1< func3n, test::peek_data >();
      test_callables();
      unit_test_2< func2 >();
      unit_test_2< func3n >();
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
