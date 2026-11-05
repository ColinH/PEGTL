// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <string>
#include <string_view>

#include "test.hpp"
#include "verify_rule.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   namespace literal_actions
   {
      template< char Kind >
      struct record
      {
         static void apply0( std::string& calls )
         {
            calls += Kind;
         }
      };

      // Native and character-pack rules must allow independent specializations.
      // clang-format off
      template< typename Rule > struct action : nothing< Rule > {};
      template< auto Text > struct action< string< Text > > : record< 's' > {};
      template< char... Cs > struct action< string_chars< Cs... > > : record< 'S' > {};
      template< auto Text > struct action< istring< Text > > : record< 'i' > {};
      template< char... Cs > struct action< istring_chars< Cs... > > : record< 'I' > {};
      template< std::size_t Prefix, auto Text > struct action< astring< Prefix, Text > > : record< 'a' > {};
      template< std::size_t Prefix, char... Cs > struct action< astring_chars< Prefix, Cs... > > : record< 'A' > {};
      template< std::size_t Prefix, auto Text > struct action< aistring< Prefix, Text > > : record< 'c' > {};
      template< std::size_t Prefix, char... Cs > struct action< aistring_chars< Prefix, Cs... > > : record< 'C' > {};
      template< auto Text > struct action< keyword< Text > > : record< 'k' > {};
      template< char... Cs > struct action< keyword_chars< Cs... > > : record< 'K' > {};
      // clang-format on

      using grammar = seq< string< "a" >, string_chars< 'b' >, istring< "c" >, istring_chars< 'd' >, astring< 1, "ef" >, astring_chars< 1, 'g', 'h' >, aistring< 1, "ij" >, aistring_chars< 1, 'k', 'l' >, keyword< "if" >, one< ' ' >, keyword_chars< 'd', 'o' >, eof >;

      void verify( const std::string_view text )
      {
         view_input in( text );
         std::string calls;
         TAO_PEGTL_TEST_ASSERT( parse< grammar, action >( in, calls ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
         TAO_PEGTL_TEST_ASSERT( calls == "sSiIaAcCkK" );
      }
   }  // namespace literal_actions

   void unit_test()
   {
      literal_actions::verify( "abCDefghIJKLif do" );
      literal_actions::verify( "abCDegIKif do" );  // Abbreviated astring/aistring matches.

      // Keep the pattern below Clang's default fold-expression nesting limit.
      static constexpr char long_text[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                          "b";
      constexpr std::size_t length = sizeof( long_text ) - 1;
      static_assert( length < 256 );

      std::string text( length - 1, 'a' );
      text += 'b';
      verify_rule< string< long_text > >( __LINE__, __FILE__, text, result_type::success, 0 );
      text.back() = 'c';
      verify_rule< string< long_text > >( __LINE__, __FILE__, text, result_type::local_failure, length );

      // Preserve byte values and a trailing null that is part of the pattern.
      verify_rule< string< "a\xff\0" > >( __LINE__, __FILE__, std::string( "a\xff\0", 3 ), result_type::success, 0 );
      verify_rule< string< "a\xff\0" > >( __LINE__, __FILE__, std::string( "a\xff", 2 ), result_type::local_failure, 2 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
