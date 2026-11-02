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

#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "test.hpp"

#include <tao/pegtl/action/builders.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   using scalar = std::variant< int, std::string, bool >;

   struct child
   {
      int value = 0;

      bool operator==( const child& ) const = default;
   };

   using composite = std::variant< int, child, std::pair< std::string, int >, bool >;
   using duplicate = std::variant< int, int >;

   struct test_target
   {
      scalar single;
      std::vector< scalar > values;
      composite composite_single;
      std::string key;
      composite named_value;
      duplicate duplicate_value;

      void set_named( std::string in_key, composite value )
      {
         key = std::move( in_key );
         named_value = std::move( value );
      }
   };

   struct yes
      : string< "yes" >
   {};

   struct no
      : string< "no" >
   {};

   struct number
      : plus< digit >
   {};

   struct word
      : plus< alpha >
   {};

   struct scalar_rule
      : sor< yes, no, number, word >
   {};

   struct values_rule
      : list< scalar_rule, one< ',' > >
   {};

   struct bad_rule
      : seq< number, one< '!' > >
   {};

   struct child_rule
      : seq< one< '[' >, number, one< ']' > >
   {};

   struct pair_name
      : plus< alpha >
   {};

   struct pair_number
      : plus< digit >
   {};

   struct pair_rule
      : seq< pair_name, one< '=' >, pair_number >
   {};

   struct composite_rule
      : sor< yes, no, child_rule, pair_rule, number >
   {};

   struct key
      : plus< alpha >
   {};

   struct named_rule
      : seq< key, one< ':' >, composite_rule, eof >
   {};

   struct positive
      : plus< digit >
   {};

   struct negative
      : seq< one< '-' >, plus< digit > >
   {};

   struct duplicate_rule
      : sor< negative, positive >
   {};

   template< typename Rule >
   struct child_action
      : nothing< Rule >
   {};

   template<>
   struct child_action< number >
      : value_to< &child::value >
   {};

   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template<>
   struct test_action< scalar_rule >
      : variant_to< &test_target::single,
                    number,
                    word,
                    cases< case_< yes, true >,
                           case_< no, false > > >
   {};

   template<>
   struct test_action< values_rule >
      : repeat_to< &test_target::values,
                   variant_for< scalar_rule,
                                number,
                                word,
                                cases< case_< yes, true >,
                                       case_< no, false > > > >
   {};

   template<>
   struct test_action< bad_rule >
      : variant_to< &test_target::single,
                    number,
                    word,
                    cases< case_< yes, true >,
                           case_< no, false > > >
   {};

   template<>
   struct test_action< composite_rule >
      : variant_to< &test_target::composite_single,
                    number,
                    create_for< child_rule, child, child_action >,
                    multi_for< pair_rule, pair_name, pair_number >,
                    cases< case_< yes, true >,
                           case_< no, false > > >
   {};

   template<>
   struct test_action< named_rule >
      : multi_to< &test_target::set_named,
                  key,
                  variant_for< composite_rule,
                               number,
                               create_for< child_rule, child, child_action >,
                               multi_for< pair_rule, pair_name, pair_number >,
                               cases< case_< yes, true >,
                                      case_< no, false > > > >
   {};

   template<>
   struct test_action< duplicate_rule >
      : variant_to< &test_target::duplicate_value,
                    positive,
                    negative >
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
      test_target word_target;
      parse_into< scalar_rule >( "alpha", word_target );
      TAO_PEGTL_TEST_ASSERT( word_target.single == scalar( std::string( "alpha" ) ) );

      test_target bool_target;
      parse_into< scalar_rule >( "yes", bool_target );
      TAO_PEGTL_TEST_ASSERT( bool_target.single == scalar( true ) );

      test_target number_target;
      parse_into< scalar_rule >( "42", number_target );
      TAO_PEGTL_TEST_ASSERT( number_target.single == scalar( 42 ) );

      test_target repeated;
      parse_into< values_rule >( "yes,42,beta,no", repeated );
      TAO_PEGTL_TEST_ASSERT( repeated.values.size() == 4 );
      TAO_PEGTL_TEST_ASSERT( repeated.values[ 0 ] == scalar( true ) );
      TAO_PEGTL_TEST_ASSERT( repeated.values[ 1 ] == scalar( 42 ) );
      TAO_PEGTL_TEST_ASSERT( repeated.values[ 2 ] == scalar( std::string( "beta" ) ) );
      TAO_PEGTL_TEST_ASSERT( repeated.values[ 3 ] == scalar( false ) );

      test_target with_child;
      parse_into< composite_rule >( "[7]", with_child );
      TAO_PEGTL_TEST_ASSERT( with_child.composite_single == composite( child{ 7 } ) );

      test_target with_pair;
      parse_into< composite_rule >( "alpha=3", with_pair );
      TAO_PEGTL_TEST_ASSERT( with_pair.composite_single == composite( std::pair< std::string, int >( "alpha", 3 ) ) );

      test_target with_named;
      parse_into< named_rule >( "slot:[9]", with_named );
      TAO_PEGTL_TEST_ASSERT( with_named.key == "slot" );
      TAO_PEGTL_TEST_ASSERT( with_named.named_value == composite( child{ 9 } ) );

      test_target first_duplicate;
      parse_into< duplicate_rule >( "5", first_duplicate );
      TAO_PEGTL_TEST_ASSERT( first_duplicate.duplicate_value.index() == 0 );
      TAO_PEGTL_TEST_ASSERT( std::get< 0 >( first_duplicate.duplicate_value ) == 5 );

      test_target second_duplicate;
      parse_into< duplicate_rule >( "-6", second_duplicate );
      TAO_PEGTL_TEST_ASSERT( second_duplicate.duplicate_value.index() == 1 );
      TAO_PEGTL_TEST_ASSERT( std::get< 1 >( second_duplicate.duplicate_value ) == -6 );

      test_target failed;
      failed.single = std::string( "before" );
      text_view_input< scan::lf > bad_input( "42" );

      TAO_PEGTL_TEST_ASSERT( !parse< bad_rule, test_action >( bad_input, failed ) );
      TAO_PEGTL_TEST_ASSERT( failed.single == scalar( std::string( "before" ) ) );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
