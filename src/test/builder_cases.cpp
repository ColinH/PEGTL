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

#include <string_view>
#include <variant>
#include <vector>

#include "test.hpp"

#include <tao/pegtl/action/builders.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   enum class answer
   {
      no,
      yes
   };

   struct test_target
   {
      answer direct = answer::no;
      bool nested = false;
      int number = 0;
      bool plain = false;
      int plain_number = 0;
      std::vector< bool > repeated;
      std::variant< bool, int > selected;

      void set_nested( const bool value, const int in_number ) noexcept
      {
         nested = value;
         number = in_number;
      }

      void set_plain( const bool value, const int in_number ) noexcept
      {
         plain = value;
         plain_number = in_number;
      }
   };

   struct direct_yes
      : string< "yes" >
   {};

   struct direct_no
      : string< "no" >
   {};

   struct direct_choice
      : sor< direct_yes, direct_no >
   {};

   struct yes
      : string< "yes" >
   {};

   struct no
      : string< "no" >
   {};

   struct choice
      : sor< yes, no >
   {};

   struct number
      : plus< digit >
   {};

   struct nested_rule
      : seq< choice, one< ':' >, number, eof >
   {};

   struct plain_rule
      : seq< choice, one< '=' >, number, eof >
   {};

   struct repeated_rule
      : list< choice, one< ',' > >
   {};

   struct selected_rule
      : sor< choice, number >
   {};

   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template<>
   struct test_action< direct_choice >
      : cases_to< &test_target::direct,
                  case_< direct_yes, answer::yes >,
                  case_< direct_no, answer::no > >
   {};

   template<>
   struct test_action< nested_rule >
      : multi_to< &test_target::set_nested,
                  cases_for< choice,
                             case_< yes, true >,
                             case_< no, false > >,
                  number >
   {};

   template<>
   struct test_action< plain_rule >
      : multi_to< &test_target::set_plain,
                  cases< case_< yes, true >,
                         case_< no, false > >,
                  number >
   {};

   template<>
   struct test_action< repeated_rule >
      : repeat_to< &test_target::repeated,
                   cases< case_< yes, true >,
                          case_< no, false > > >
   {};

   template<>
   struct test_action< selected_rule >
      : variant_to< &test_target::selected,
                    cases< case_< yes, true >,
                           case_< no, false > >,
                    number >
   {};

   template< typename Rule >
   void parse_into( const std::string_view text, test_target& target )
   {
      text_view_input< scan::lf > in( text );
      TAO_PEGTL_TEST_ASSERT( parse< Rule, test_action >( in, target ) );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

   void unit_test()
   {
      test_target target;

      parse_into< direct_choice >( "yes", target );
      parse_into< nested_rule >( "yes:42", target );
      parse_into< plain_rule >( "no=7", target );
      parse_into< repeated_rule >( "yes,no,yes", target );
      parse_into< selected_rule >( "23", target );

      TAO_PEGTL_TEST_ASSERT( target.direct == answer::yes );
      TAO_PEGTL_TEST_ASSERT( target.nested );
      TAO_PEGTL_TEST_ASSERT( target.number == 42 );
      TAO_PEGTL_TEST_ASSERT( !target.plain );
      TAO_PEGTL_TEST_ASSERT( target.plain_number == 7 );
      TAO_PEGTL_TEST_ASSERT( target.repeated == std::vector< bool >( { true, false, true } ) );
      TAO_PEGTL_TEST_ASSERT( target.selected == std::variant< bool, int >( 23 ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
