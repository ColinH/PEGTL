// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/extra/type_to_string.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Literal, typename Chars >
   void verify_strings( const std::string_view expected )
   {
      static_assert( type_to_string_view< Literal >() == type_to_string_view< Chars >() );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< Literal >() == expected );
      TAO_PEGTL_TEST_ASSERT( type_to_string< Literal >() == expected );
      TAO_PEGTL_TEST_ASSERT( type_to_string< Chars >() == expected );
   }

   void unit_test()
   {
      TAO_PEGTL_TEST_ASSERT( type_to_string<>().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string< 'a', 'b', 'c' >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string< string_chars<> >().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string< string_chars< 'a', 'b', 'c' > >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string< istring_chars<> >().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string< istring_chars< 'a', 'b', 'c' > >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string< string< "" > >().empty() );
      TAO_PEGTL_TEST_ASSERT( type_to_string< string< "abc" > >() == "abc" );
      TAO_PEGTL_TEST_ASSERT( type_to_string< string< "AbC" > >() == "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string< string< "abc" > >() != "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string< istring< "abc" > >() == "abc" );
      TAO_PEGTL_TEST_ASSERT( type_to_string< istring< "AbC" > >() == "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string< istring< "abc" > >() != "AbC" );

      TAO_PEGTL_TEST_ASSERT( ( type_to_string< one< 'a', 'b', 'c' > >() == "abc" ) );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string< not_one7< 'a', 'b', 'c' > >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string_view<>().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string_view< 'a', 'b', 'c' >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string_view< string_chars<> >().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string_view< string_chars< 'a', 'b', 'c' > >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string_view< istring_chars<> >().empty() );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string_view< istring_chars< 'a', 'b', 'c' > >() == "abc" ) );

      TAO_PEGTL_TEST_ASSERT( type_to_string_view< string< "" > >().empty() );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< string< "abc" > >() == "abc" );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< string< "AbC" > >() == "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< string< "abc" > >() != "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< istring< "abc" > >() == "abc" );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< istring< "AbC" > >() == "AbC" );
      TAO_PEGTL_TEST_ASSERT( type_to_string_view< istring< "abc" > >() != "AbC" );

      TAO_PEGTL_TEST_ASSERT( ( type_to_string_view< one< 'a', 'b', 'c' > >() == "abc" ) );
      TAO_PEGTL_TEST_ASSERT( ( type_to_string_view< not_one7< 'a', 'b', 'c' > >() == "abc" ) );

      verify_strings< keyword< "if" >, keyword_chars< 'i', 'f' > >( "if" );
      verify_strings< astring< 1, "abc" >, astring_chars< 1, 'a', 'b', 'c' > >( "abc" );
      verify_strings< aistring< 1, "AbC" >, aistring_chars< 1, 'A', 'b', 'C' > >( "AbC" );
      verify_strings< string< "a\0b\0" >, string_chars< 'a', '\0', 'b', '\0' > >( std::string_view( "a\0b\0", 4 ) );
      verify_strings< istring< "a\0b\0" >, istring_chars< 'a', '\0', 'b', '\0' > >( std::string_view( "a\0b\0", 4 ) );
      verify_strings< astring< 1, "a\0b\0" >, astring_chars< 1, 'a', '\0', 'b', '\0' > >( std::string_view( "a\0b\0", 4 ) );
      verify_strings< aistring< 1, "a\0b\0" >, aistring_chars< 1, 'a', '\0', 'b', '\0' > >( std::string_view( "a\0b\0", 4 ) );
#if defined( __cpp_exceptions )
      verify_strings< raise_message< "error" >, raise_message_chars< 'e', 'r', 'r', 'o', 'r' > >( "error" );
      verify_strings< raise_message< "" >, raise_message_chars<> >( "" );
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
