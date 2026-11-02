// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#if !defined( __cpp_exceptions )
#include <iostream>
int main()
{
   std::cout << "Exception support disabled, skipping test..." << std::endl;
}
#else

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "test.hpp"

#include <tao/pegtl/action/builders.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct child
   {
      int value = 10;

      bool operator==( const child& ) const = default;
   };

   struct test_target
   {
      int constant = 1;
      std::string value = "before";
      int choice = 2;
      std::string name = "before";
      int number = 3;
      std::optional< int > optional = 4;
      std::vector< int > repeated = { 5 };
      std::variant< int, std::string > variant = std::string( "before" );
      child created;
      int suppressed = 6;

      void set_multi( std::string in_name, const int in_number )
      {
         name = std::move( in_name );
         number = in_number;
      }
   };

   struct const_rule
      : one< 'c' >
   {};

   struct value_rule
      : plus< alpha >
   {};

   struct yes
      : string< "yes" >
   {};

   struct no
      : string< "no" >
   {};

   struct cases_rule
      : sor< yes, no >
   {};

   struct multi_name
      : plus< alpha >
   {};

   struct multi_number
      : plus< digit >
   {};

   struct multi_rule
      : seq< multi_name, one< ':' >, multi_number >
   {};

   struct optional_number
      : plus< digit >
   {};

   struct optional_rule
      : opt< optional_number >
   {};

   struct repeat_number
      : plus< digit >
   {};

   struct repeat_rule
      : list< repeat_number, one< ',' > >
   {};

   struct variant_number
      : plus< digit >
   {};

   struct variant_word
      : plus< alpha >
   {};

   struct variant_rule
      : sor< variant_number, variant_word >
   {};

   struct create_number
      : plus< digit >
   {};

   struct create_rule
      : seq< one< '[' >, create_number, one< ']' > >
   {};

   struct suppressed_inner
      : one< 'x' >
   {};

   struct suppressing_number
      : plus< digit >
   {};

   struct suppressing_rule
      : seq< suppressed_inner, suppressing_number, eof >
   {};

   template< typename Rule >
   struct child_action
      : nothing< Rule >
   {};

   template<>
   struct child_action< create_number >
      : value_to< &child::value >
   {};

   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template<>
   struct test_action< const_rule >
      : const_to< &test_target::constant, 11 >
   {};

   template<>
   struct test_action< value_rule >
      : value_to< &test_target::value >
   {};

   template<>
   struct test_action< cases_rule >
      : cases_to< &test_target::choice,
                  case_< yes, 12 >,
                  case_< no, 13 > >
   {};

   template<>
   struct test_action< multi_rule >
      : multi_to< &test_target::set_multi, multi_name, multi_number >
   {};

   template<>
   struct test_action< optional_rule >
      : optional_to< &test_target::optional, optional_number >
   {};

   template<>
   struct test_action< repeat_rule >
      : repeat_to< &test_target::repeated, repeat_number >
   {};

   template<>
   struct test_action< variant_rule >
      : variant_to< &test_target::variant, variant_number, variant_word >
   {};

   template<>
   struct test_action< create_rule >
      : create_to< &test_target::created, child, child_action >
   {};

   template<>
   struct test_action< suppressed_inner >
      : const_to< &test_target::suppressed, 19 >
   {};

   template<>
   struct test_action< suppressing_rule >
      : multi_to< &test_target::number, suppressing_number >
   {};

   template< typename Rule >
   void parse_disabled( const std::string_view text, test_target& target )
   {
      text_view_input< scan::lf > in( text );
      TAO_PEGTL_TEST_ASSERT( ( parse< Rule, test_action, normal, apply_mode::disabled >( in, target ) ) );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

   void unit_test()
   {
      test_target target;

      parse_disabled< const_rule >( "c", target );
      parse_disabled< value_rule >( "changed", target );
      parse_disabled< cases_rule >( "yes", target );
      parse_disabled< multi_rule >( "changed:14", target );
      parse_disabled< optional_rule >( "15", target );
      parse_disabled< repeat_rule >( "16,17", target );
      parse_disabled< variant_rule >( "changed", target );
      parse_disabled< create_rule >( "[18]", target );

      TAO_PEGTL_TEST_ASSERT( target.constant == 1 );
      TAO_PEGTL_TEST_ASSERT( target.value == "before" );
      TAO_PEGTL_TEST_ASSERT( target.choice == 2 );
      TAO_PEGTL_TEST_ASSERT( target.name == "before" );
      TAO_PEGTL_TEST_ASSERT( target.number == 3 );
      TAO_PEGTL_TEST_ASSERT( target.optional == 4 );
      TAO_PEGTL_TEST_ASSERT( target.repeated == std::vector< int >( { 5 } ) );
      TAO_PEGTL_TEST_ASSERT( target.variant == std::variant< int, std::string >( std::string( "before" ) ) );
      TAO_PEGTL_TEST_ASSERT( target.created == child{ 10 } );

      text_view_input< scan::lf > suppressing_input( "x20" );
      TAO_PEGTL_TEST_ASSERT( parse< suppressing_rule, test_action >( suppressing_input, target ) );
      TAO_PEGTL_TEST_ASSERT( suppressing_input.empty() );
      TAO_PEGTL_TEST_ASSERT( target.number == 20 );
      TAO_PEGTL_TEST_ASSERT( target.suppressed == 6 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
