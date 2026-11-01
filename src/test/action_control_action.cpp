// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "test.hpp"
#include "test_utility.hpp"

#include <tao/pegtl/action/control_action.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template< typename Rule >
   struct dummy_action
      : control_action
   {};

   struct first_rule
      : sor< alpha, digit >
   {};

#if defined( __cpp_exceptions )
   struct second_rule
      : must< alnum >
   {};
#endif

   std::string story;

   template<>
   struct test_action< first_rule >
      : control_action
   {
      template< typename ParseInput >
      static void start( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'a';
      }

      template< typename ParseInput >
      static void success( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'b';
      }

      // LCOV_EXCL_START
      template< typename ParseInput >
      static void failure( const ParseInput& /*unused*/, int /*unused*/ )
      {
         TAO_PEGTL_TEST_UNREACHABLE;
      }
      // LCOV_EXCL_STOP
   };

   template<>
   struct test_action< alpha >
      : control_action
   {
      template< typename ParseInput >
      static void start( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'd';
      }

      // LCOV_EXCL_START
      template< typename ParseInput >
      static void success( const ParseInput& /*unused*/, int /*unused*/ )
      {
         TAO_PEGTL_TEST_UNREACHABLE;
      }
      // LCOV_EXCL_STOP

      template< typename ParseInput >
      static void failure( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'f';
      }
   };

   template<>
   struct test_action< digit >
      : control_action
   {
      template< typename ParseInput >
      static void start( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'g';
      }

      template< typename ParseInput >
      static void success( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'h';
      }

      // LCOV_EXCL_START
      template< typename ParseInput >
      static void failure( const ParseInput& /*unused*/, int /*unused*/ )
      {
         TAO_PEGTL_TEST_UNREACHABLE;
      }
      // LCOV_EXCL_STOP
   };

#if defined( __cpp_exceptions )
   template<>
   struct test_action< second_rule >
      : control_action
   {
      template< typename ParseInput >
      static void start( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'j';
      }

      // LCOV_EXCL_START
      template< typename ParseInput >
      static void success( const ParseInput& /*unused*/, int /*unused*/ )
      {
         TAO_PEGTL_TEST_UNREACHABLE;
      }

      template< typename ParseInput >
      static void failure( const ParseInput& /*unused*/, int /*unused*/ )
      {
         TAO_PEGTL_TEST_UNREACHABLE;
      }
      // LCOV_EXCL_STOP

      template< typename ParseInput >
      static void unwind( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'm';
      }
   };

   template< typename Rule >
   struct nonvoid_unwind_action
      : test_action< Rule >
   {};

   template<>
   struct nonvoid_unwind_action< second_rule >
      : test_action< second_rule >
   {
      template< typename ParseInput >
      static bool unwind( const ParseInput& /*unused*/, int /*unused*/ )
      {
         story += 'n';
         return false;
      }
   };
#endif

   void unit_test()
   {
      {
         text_view_input< scan::lf > in( "0" );
         const auto b = parse< first_rule, dummy_action >( in, 42 );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( story.empty() );
      }
      {
         text_view_input< scan::lf > in( "0" );
         const auto b = parse< first_rule, test_action >( in, 42 );
         TAO_PEGTL_TEST_ASSERT( b );
         TAO_PEGTL_TEST_ASSERT( story == "adfghb" );
      }
#if defined( __cpp_exceptions )
      story.clear();
      {
         text_view_input< scan::lf > in( "*" );
         TAO_PEGTL_TEST_THROWS( parse< second_rule, test_action >( in, 42 ) );
         TAO_PEGTL_TEST_ASSERT( story == "jm" );
      }
      story.clear();
      {
         text_view_input< scan::lf > in( "*" );
         TAO_PEGTL_TEST_THROWS( parse< second_rule, nonvoid_unwind_action >( in, 42 ) );
         TAO_PEGTL_TEST_ASSERT( story == "jn" );
      }
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
