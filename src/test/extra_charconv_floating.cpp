// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <tao/pegtl/extra/internal/charconv_features.hpp>

#if !defined( __cpp_exceptions ) || !defined( TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS )
#include <iostream>
int main()
{
   std::cout << "Exception support or floating-point from_chars unavailable, skipping test..." << std::endl;
}
#else

#include "test.hpp"
#include "verify_meta.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include <tao/pegtl/extra/builders.hpp>
#include <tao/pegtl/extra/charconv.hpp>
#include <tao/pegtl/stream.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Number, std::chars_format Format, overflow_mode Over, typename Matched = everything >
   struct floating_action
   {
      template< typename Rule >
      using type = std::conditional_t< std::is_same_v< Rule, Matched >, from_fchars_combo< Number, Format, Over >, nothing< Rule > >;
   };

   template< typename Floating >
   bool same_floating( const Floating lhs, const Floating rhs )
   {
      return std::isnan( rhs ) ? std::isnan( lhs ) : ( lhs == rhs && std::signbit( lhs ) == std::signbit( rhs ) );
   }

   template< typename Rule, typename Floating >
   void floating_rule_success( const std::string& input, const Floating output, const std::size_t remaining )
   {
      view_input in( input );
      Floating state = 42;
      int other = 17;
      TAO_PEGTL_TEST_ASSERT( parse< Rule >( in, state, other ) );
      TAO_PEGTL_TEST_ASSERT( same_floating( state, output ) );
      TAO_PEGTL_TEST_ASSERT( in.size() == remaining );
      TAO_PEGTL_TEST_ASSERT( other == 17 );
   }

   template< typename Number, std::chars_format Format, overflow_mode Over, typename Floating >
   void floating_action_success( const std::string& input, const Floating output )
   {
      view_input in( input );
      Floating state = 42;
      TAO_PEGTL_TEST_ASSERT( parse< everything, floating_action< Number, Format, Over >::template type >( in, state ) );
      TAO_PEGTL_TEST_ASSERT( same_floating( state, output ) );
      TAO_PEGTL_TEST_ASSERT( in.empty() );
   }

   template< typename Floating, std::chars_format Format = std::chars_format::general >
   void floating_success( const std::string& input, const Floating output, const std::size_t remaining = 0 )
   {
      floating_rule_success< from_fchars_nothrow< Floating, Format > >( input, output, remaining );
      floating_rule_success< from_fchars_throws< Floating, Format > >( input, output, remaining );
      floating_rule_success< from_fchars_nothrow< void, Format > >( input, output, remaining );
      floating_rule_success< from_fchars_throws< void, Format > >( input, output, remaining );

      const auto matched = input.substr( 0, input.size() - remaining );
      floating_action_success< Floating, Format, overflow_mode::local_failure >( matched, output );
      floating_action_success< Floating, Format, overflow_mode::global_failure >( matched, output );
      floating_action_success< void, Format, overflow_mode::local_failure >( matched, output );
      floating_action_success< void, Format, overflow_mode::global_failure >( matched, output );
   }

   template< typename Rule, typename Floating >
   void floating_rule_failure( const std::string& input, const bool throws )
   {
      view_input in( input );
      Floating state = 42;
      if( throws ) {
         try {
            ( void )parse< Rule >( in, state );
            TAO_PEGTL_TEST_ASSERT( false );
         }
         catch( const parse_error_base& ) {
         }
      }
      else {
         TAO_PEGTL_TEST_ASSERT( !parse< Rule >( in, state ) );
      }
      TAO_PEGTL_TEST_ASSERT( state == 42 );
      TAO_PEGTL_TEST_ASSERT( in.size() == input.size() );
   }

   template< typename Number, std::chars_format Format, overflow_mode Over, typename Floating >
   void floating_action_failure( const std::string& input, const bool throws )
   {
      view_input in( input );
      Floating state = 42;
      if( throws ) {
         try {
            ( void )parse< everything, floating_action< Number, Format, Over >::template type >( in, state );
            TAO_PEGTL_TEST_ASSERT( false );
         }
         catch( const parse_error_base& ) {
         }
      }
      else {
         TAO_PEGTL_TEST_ASSERT( !parse< everything, floating_action< Number, Format, Over >::template type >( in, state ) );
      }
      TAO_PEGTL_TEST_ASSERT( state == 42 );
      TAO_PEGTL_TEST_ASSERT( in.size() == input.size() );
   }

   template< typename Floating, std::chars_format Format = std::chars_format::general >
   void floating_failure( const std::string& input, const bool range_error = false )
   {
      floating_rule_failure< from_fchars_nothrow< Floating, Format >, Floating >( input, false );
      floating_rule_failure< from_fchars_throws< Floating, Format >, Floating >( input, range_error );
      floating_rule_failure< from_fchars_nothrow< void, Format >, Floating >( input, false );
      floating_rule_failure< from_fchars_throws< void, Format >, Floating >( input, range_error );

      floating_action_failure< Floating, Format, overflow_mode::local_failure, Floating >( input, false );
      floating_action_failure< Floating, Format, overflow_mode::global_failure, Floating >( input, range_error );
      floating_action_failure< void, Format, overflow_mode::local_failure, Floating >( input, false );
      floating_action_failure< void, Format, overflow_mode::global_failure, Floating >( input, range_error );
   }

   template< typename Floating >
   void test_floating()
   {
      floating_success( "0", Floating( 0 ) );
      floating_success( "-0", Floating( -0.0 ) );
      floating_success( ".5", Floating( 0.5 ) );
      floating_success( "1.", Floating( 1 ) );
      floating_success( "-1.25", Floating( -1.25 ) );
      floating_success( "1e2", Floating( 100 ) );
      floating_success( "1E+2", Floating( 100 ) );
      floating_success( "125e-3", Floating( 0.125 ) );
      floating_success( "1.25 rest", Floating( 1.25 ), 5 );
      floating_success( std::string( "1.25\0rest", 9 ), Floating( 1.25 ), 5 );
      floating_success( "1e+", Floating( 1 ), 2 );
      floating_success( "0x1p2", Floating( 0 ), 4 );

      floating_success( "inf", std::numeric_limits< Floating >::infinity() );
      floating_success( "-INFINITY", -std::numeric_limits< Floating >::infinity() );
      floating_success( "infinite", std::numeric_limits< Floating >::infinity(), 5 );
      floating_success( "nan", std::numeric_limits< Floating >::quiet_NaN() );
      floating_success( "-NAN()", std::numeric_limits< Floating >::quiet_NaN() );
      floating_success( "NaN(payload_123)", std::numeric_limits< Floating >::quiet_NaN() );

      floating_failure< Floating >( "" );
      floating_failure< Floating >( " 1" );
      floating_failure< Floating >( "+1" );
      floating_failure< Floating >( "-" );
      floating_failure< Floating >( "." );
      floating_failure< Floating >( "text" );
      floating_failure< Floating >( "1e9999", true );
      floating_failure< Floating >( "-1e9999", true );
      floating_failure< Floating >( "1e-9999", true );
      floating_action_failure< Floating, std::chars_format::general, overflow_mode::local_failure, Floating >( "1.25x", false );
      floating_action_failure< void, std::chars_format::general, overflow_mode::global_failure, Floating >( "1.25x", false );

      floating_success< Floating, std::chars_format::fixed >( "1.25e2", Floating( 1.25 ), 2 );
      floating_success< Floating, std::chars_format::fixed >( "inf", std::numeric_limits< Floating >::infinity() );
      floating_success< Floating, std::chars_format::scientific >( "1.25e2", Floating( 125 ) );
      floating_success< Floating, std::chars_format::scientific >( "nan", std::numeric_limits< Floating >::quiet_NaN() );
      floating_failure< Floating, std::chars_format::scientific >( "1.25" );
      floating_failure< Floating, std::chars_format::scientific >( "1e+" );

      floating_success< Floating, std::chars_format::hex >( "1.8p+2", Floating( 6 ) );
      floating_success< Floating, std::chars_format::hex >( "-1.8P-2", Floating( -0.375 ) );
      floating_success< Floating, std::chars_format::hex >( "1.8", Floating( 1.5 ) );
      floating_success< Floating, std::chars_format::hex >( "1e2", Floating( 482 ) );
      floating_success< Floating, std::chars_format::hex >( "0x1p2", Floating( 0 ), 4 );
      floating_success< Floating, std::chars_format::hex >( "inf", std::numeric_limits< Floating >::infinity() );
      floating_failure< Floating, std::chars_format::hex >( "g" );
      floating_failure< Floating, std::chars_format::hex >( "1p99999", true );
      floating_failure< Floating, std::chars_format::hex >( "1p-99999", true );

      floating_success( std::string( 10000, '0' ) + "1", Floating( 1 ) );
      floating_success( "1." + std::string( 10000, '0' ), Floating( 1 ) );
      floating_success( "1e" + std::string( 10000, '0' ) + "2", Floating( 100 ) );
      floating_failure< Floating >( std::string( 10000, '9' ), true );

      {
         view_input in( "1.25" );
         TAO_PEGTL_TEST_ASSERT( parse< from_fchars_nothrow< Floating > >( in ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "1.25" );
         int other = 17;
         Floating state = 42;
         TAO_PEGTL_TEST_ASSERT( parse< from_fchars_nothrow< Floating > >( in, other, state ) );
         TAO_PEGTL_TEST_ASSERT( other == 17 );
         TAO_PEGTL_TEST_ASSERT( state == 42 );
      }
      {
         view_input in( "1.25" );
         Floating state = 42;
         TAO_PEGTL_TEST_ASSERT( parse< disable< from_fchars_nothrow< Floating > > >( in, state ) );
         TAO_PEGTL_TEST_ASSERT( state == 42 );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "1.25" );
         Floating state = 42;
         TAO_PEGTL_TEST_ASSERT( parse< disable< from_fchars_nothrow< void > > >( in, state ) );
         TAO_PEGTL_TEST_ASSERT( state == Floating( 1.25 ) );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         alloc_cstring_input< void > in( 16, 1, "1.25" );
         Floating state = 42;
         struct rule : rep< 4, any >
         {};
         TAO_PEGTL_TEST_ASSERT( parse< rule, floating_action< Floating, std::chars_format::general, overflow_mode::local_failure, rule >::template type >( in, state ) );
         TAO_PEGTL_TEST_ASSERT( state == Floating( 1.25 ) );
      }
   }

   struct other_state
   {};

   struct floating_target
   {
      float value = 0;
      double first = 0;
      float second = 0;

      void set_pair( const double in_first, const float in_second ) noexcept
      {
         first = in_first;
         second = in_second;
      }
   };

   struct floating_rule
      : from_fchars_nothrow< float >
   {};

   struct floating_void_rule
      : from_fchars_nothrow< void, std::chars_format::scientific >
   {};

   struct floating_throws_rule
      : from_fchars_throws< double >
   {};

   struct floating_pair_rule
      : seq< floating_throws_rule, one< ',' >, floating_void_rule, eof >
   {};

   template< typename Rule >
   struct floating_builder_action
      : nothing< Rule >
   {};

   template<>
   struct floating_builder_action< floating_rule >
      : from_chars_to< &floating_target::value >
   {};

   template<>
   struct floating_builder_action< floating_void_rule >
      : from_chars_to< &floating_target::value >
   {};

   template<>
   struct floating_builder_action< floating_throws_rule >
      : from_chars_to< &floating_target::first >
   {};

   template<>
   struct floating_builder_action< floating_pair_rule >
      : multi_to< &floating_target::set_pair,
                  from_chars_for< floating_throws_rule >,
                  from_chars_for< floating_void_rule > >
   {};

   void test_floating_builders()
   {
      other_state other;
      floating_target target;
      {
         view_input in( "1.25" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_rule, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 1.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "25e-2" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_void_rule, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      {
         view_input in( "1.25,25e-2" );
         TAO_PEGTL_TEST_ASSERT( parse< floating_pair_rule, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.first == 1.25 );
         TAO_PEGTL_TEST_ASSERT( target.second == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
      for( const auto* input : { "invalid", "1e9999", "1e-9999" } ) {
         view_input in( std::string_view{ input } );
         TAO_PEGTL_TEST_ASSERT( !parse< floating_rule, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.current() == input );
      }
      {
         view_input in( "1e9999" );
         TAO_PEGTL_TEST_THROWS( parse< floating_throws_rule, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.first == 1.25 );
      }
      {
         view_input in( "2.5" );
         TAO_PEGTL_TEST_ASSERT( parse< disable< floating_rule >, floating_builder_action >( in, other, target ) );
         TAO_PEGTL_TEST_ASSERT( target.value == 0.25F );
         TAO_PEGTL_TEST_ASSERT( in.empty() );
      }
   }

   void unit_test()
   {
      verify_analyze< from_fchars_throws< float > >( __LINE__, __FILE__, true, false );
      verify_analyze< from_fchars_nothrow< double > >( __LINE__, __FILE__, true, false );
      verify_analyze< from_fchars_throws< void > >( __LINE__, __FILE__, true, false );
      verify_analyze< from_fchars_nothrow< void, std::chars_format::hex > >( __LINE__, __FILE__, true, false );

      test_floating< float >();
      test_floating< double >();
      test_floating_builders();

      floating_success( "3.4028234663852886e38", ( std::numeric_limits< float >::max )() );
      floating_success( "1.7976931348623157e308", ( std::numeric_limits< double >::max )() );
      floating_success( "1.1754943508222875e-38", ( std::numeric_limits< float >::min )() );
      floating_success( "2.2250738585072014e-308", ( std::numeric_limits< double >::min )() );
      floating_success( "1.401298464324817e-45", std::numeric_limits< float >::denorm_min() );
      floating_success( "4.9406564584124654e-324", std::numeric_limits< double >::denorm_min() );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
