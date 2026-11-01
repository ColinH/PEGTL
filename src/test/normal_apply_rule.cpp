// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule >
   struct my_action
      : nothing< Rule >
   {};

   struct a
      : one< 'a' >
   {};

   struct ar
      : one< 'r' >
   {};

   struct a0
      : one< '0' >
   {};

   struct a0r
      : one< '1' >
   {};

   struct ab
      : one< 'A' >
   {};

   struct arb
      : one< 'R' >
   {};

   struct a0b
      : one< '2' >
   {};

   struct a0rb
      : one< '3' >
   {};

   struct af
      : one< 'F' >
   {};

   struct arf
      : one< 'G' >
   {};

   struct a0f
      : one< '4' >
   {};

   struct a0rf
      : one< '5' >
   {};

   std::string accumulator;

   template<>
   struct my_action< a >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& /*unused*/ ) noexcept
      {
         accumulator += 'a';
      }
   };

   template<>
   struct my_action< ar >
   {
      template< typename Rule, typename ActionInput >
      static void apply( const ActionInput& /*unused*/ ) noexcept
      {
         static_assert( std::is_same_v< Rule, ar > );
         accumulator += 'r';
      }
   };

   template<>
   struct my_action< a0 >
   {
      static void apply0() noexcept
      {
         accumulator += '0';
      }
   };

   template<>
   struct my_action< a0r >
   {
      template< typename Rule >
      static void apply0() noexcept
      {
         static_assert( std::is_same_v< Rule, a0r > );
         accumulator += '1';
      }
   };

   template<>
   struct my_action< ab >
   {
      template< typename ActionInput >
      static bool apply( const ActionInput& /*unused*/ ) noexcept
      {
         accumulator += 'A';
         return true;
      }
   };

   template<>
   struct my_action< arb >
   {
      template< typename Rule, typename ActionInput >
      static bool apply( const ActionInput& /*unused*/ ) noexcept
      {
         static_assert( std::is_same_v< Rule, arb > );
         accumulator += 'R';
         return true;
      }
   };

   template<>
   struct my_action< a0b >
   {
      static bool apply0() noexcept
      {
         accumulator += '2';
         return true;
      }
   };

   template<>
   struct my_action< a0rb >
   {
      template< typename Rule >
      static bool apply0() noexcept
      {
         static_assert( std::is_same_v< Rule, a0rb > );
         accumulator += '3';
         return true;
      }
   };

   template<>
   struct my_action< af >
   {
      template< typename ActionInput >
      static bool apply( const ActionInput& /*unused*/ ) noexcept
      {
         accumulator += 'F';
         return false;
      }
   };

   template<>
   struct my_action< arf >
   {
      template< typename Rule, typename ActionInput >
      static bool apply( const ActionInput& /*unused*/ ) noexcept
      {
         static_assert( std::is_same_v< Rule, arf > );
         accumulator += 'G';
         return false;
      }
   };

   template<>
   struct my_action< a0f >
   {
      static bool apply0() noexcept
      {
         accumulator += '4';
         return false;
      }
   };

   template<>
   struct my_action< a0rf >
   {
      template< typename Rule >
      static bool apply0() noexcept
      {
         static_assert( std::is_same_v< Rule, a0rf > );
         accumulator += '5';
         return false;
      }
   };

   template<>
   struct my_action< one< 'F' > >
   {
      static void apply0() noexcept
      {
         accumulator += '-';
      }
   };

   template<>
   struct my_action< one< 'G' > >
   {
      static void apply0() noexcept
      {
         accumulator += '-';
      }
   };

   template<>
   struct my_action< one< '4' > >
   {
      static void apply0() noexcept
      {
         accumulator += '+';
      }
   };

   template<>
   struct my_action< one< '5' > >
   {
      static void apply0() noexcept
      {
         accumulator += '+';
      }
   };

   using my_grammar = seq< a, ar, a0, a0r, ab, arb, a0b, a0rb, sor< af, one< 'F' > >, sor< arf, one< 'G' > >, sor< a0f, one< '4' > >, sor< a0rf, one< '5' > >, eof >;

   template< typename Rule >
   struct match_override
      : nothing< Rule >
   {
      template< typename Matched, apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control, typename ParseInput >
      static int match( ParseInput& /*unused*/, int& calls )
      {
         ++calls;
         return 1;
      }
   };

   void unit_test()
   {
      const std::string str = "ar01AR23FG45";
      const std::string out = "ar01AR23F-G-4+5+";
      view_input in( str );
      const bool b = parse< my_grammar, my_action >( in );
      TAO_PEGTL_TEST_ASSERT( b );
      TAO_PEGTL_TEST_ASSERT( accumulator == out );

      // A callable action match overrides the rule even when its result is not bool.
      view_input override_input( "" );
      int calls = 0;
      TAO_PEGTL_TEST_ASSERT( parse< failure, match_override >( override_input, calls ) );
      TAO_PEGTL_TEST_ASSERT( calls == 1 );

      // An incompatible state excludes the override and leaves the rule's failure intact.
      const int const_state = 0;
      TAO_PEGTL_TEST_ASSERT( !parse< failure, match_override >( override_input, const_state ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
