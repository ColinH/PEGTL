// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <vector>

#include "test.hpp"
#include "test_endian.hpp"

#include <tao/pegtl/binary/uint16.hpp>
#include <tao/pegtl/member.hpp>
#include <tao/pegtl/unicode/utf16.hpp>
#include <tao/pegtl/unicode/utf8.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct ref_qualified_discard
   {
      int inc_rewind_guards() &&;
   };

   static_assert( internal::has_auto_discard< ref_qualified_discard > );
   static_assert( !internal::has_auto_discard< ref_qualified_discard& > );
   static_assert( !internal::has_auto_discard< view_input<> > );

   struct rewind_required_marker
   {
      using rule_t = eol_unknown_tag;
   };

   template< typename Rule >
   void verify_required_rewind( const std::size_t line, const char* file, const std::string& data, const std::size_t offset = 0 )
   {
      text_view_input< scan::lf > in( data.data(), data.data() + data.size() );
      in.template consume< rewind_required_marker >( offset );

      const auto* current = in.current();
      const auto size = in.size();

#if defined( __cpp_exceptions )
      try {
#endif
         const auto result = normal< Rule >::template match< apply_mode::enabled, rewind_mode::required, nothing, normal >( in );

         if( result ) {
            TAO_PEGTL_TEST_FAILED( "result received/expected [ success / local_failure ]" );
         }
         if( ( in.current() != current ) || ( in.size() != size ) ) {
            TAO_PEGTL_TEST_FAILED( "input was consumed on local failure, remain received/expected [ " << in.size() << " / " << size << " ]" );
         }
#if defined( __cpp_exceptions )
      }
      catch( const std::exception& e ) {
         TAO_PEGTL_TEST_FAILED( "exception instead of local failure [ " << e.what() << " ]" );
      }
#endif
   }

   struct member_token
   {
      int value;
   };

   template< typename Rule >
   void verify_required_member_rewind( const std::size_t line, const char* file, const std::vector< member_token >& data )
   {
      view_input< void, member_token > in( data );

      const auto* current = in.current();
      const auto size = in.size();

#if defined( __cpp_exceptions )
      try {
#endif
         const auto result = normal< Rule >::template match< apply_mode::enabled, rewind_mode::required, nothing, normal >( in );

         if( result ) {
            TAO_PEGTL_TEST_FAILED( "result received/expected [ success / local_failure ]" );
         }
         if( ( in.current() != current ) || ( in.size() != size ) ) {
            TAO_PEGTL_TEST_FAILED( "input was consumed on local failure, remain received/expected [ " << in.size() << " / " << size << " ]" );
         }
#if defined( __cpp_exceptions )
      }
      catch( const std::exception& e ) {
         TAO_PEGTL_TEST_FAILED( "exception instead of local failure [ " << e.what() << " ]" );
      }
#endif
   }

   template< typename Rule >
   void verify_required_string_rewind( const std::size_t line, const char* file, const std::vector< std::string >& data )
   {
      view_input< void, std::string > in( data );

      const auto* current = in.current();
      const auto size = in.size();

#if defined( __cpp_exceptions )
      try {
#endif
         const auto result = normal< Rule >::template match< apply_mode::enabled, rewind_mode::required, nothing, normal >( in );

         if( result ) {
            TAO_PEGTL_TEST_FAILED( "result received/expected [ success / local_failure ]" );
         }
         if( ( in.current() != current ) || ( in.size() != size ) ) {
            TAO_PEGTL_TEST_FAILED( "input was consumed on local failure, remain received/expected [ " << in.size() << " / " << size << " ]" );
         }
#if defined( __cpp_exceptions )
      }
      catch( const std::exception& e ) {
         TAO_PEGTL_TEST_FAILED( "exception instead of local failure [ " << e.what() << " ]" );
      }
#endif
   }

   template< typename Rule >
   void verify_required_source_rewind( const std::size_t line, const char* file, const std::string& source, const std::string& data )
   {
      text_view_input< scan::lf, char, std::string > in( source, data );

      const auto* current = in.current();
      const auto size = in.size();

#if defined( __cpp_exceptions )
      try {
#endif
         const auto result = normal< Rule >::template match< apply_mode::enabled, rewind_mode::required, nothing, normal >( in );

         if( result ) {
            TAO_PEGTL_TEST_FAILED( "result received/expected [ success / local_failure ]" );
         }
         if( ( in.current() != current ) || ( in.size() != size ) ) {
            TAO_PEGTL_TEST_FAILED( "input was consumed on local failure, remain received/expected [ " << in.size() << " / " << size << " ]" );
         }
#if defined( __cpp_exceptions )
      }
      catch( const std::exception& e ) {
         TAO_PEGTL_TEST_FAILED( "exception instead of local failure [ " << e.what() << " ]" );
      }
#endif
   }

   [[nodiscard]] bool return_false( text_view_input< scan::lf >& /*unused*/ ) noexcept
   {
      return false;
   }

   [[nodiscard]] bool char_return_false( const char /*unused*/ ) noexcept
   {
      return false;
   }

   struct false_apply
   {
      template< typename ActionInput >
      [[nodiscard]] static bool apply( const ActionInput& /*unused*/ ) noexcept
      {
         return false;
      }
   };

   struct false_apply0
   {
      [[nodiscard]] static bool apply0() noexcept
      {
         return false;
      }
   };

   struct test_state
   {
      template< typename ParseInput, typename... States >
      void success( const ParseInput& /*unused*/, States&&... /*unused*/ ) const noexcept
      {}
   };

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

   using source_inner = seq< string< "foo" >, eof >;

   [[nodiscard]] std::string u16s( const char16_t u )
   {
      return { reinterpret_cast< const char* >( &u ), sizeof( u ) };
   }

   [[nodiscard]] std::string u16s_be( const char16_t v )
   {
      const auto u = test::h_to_be( static_cast< std::uint16_t >( v ) );
      return { reinterpret_cast< const char* >( &u ), sizeof( u ) };
   }

   void test_atomic_rules()
   {
      verify_required_rewind< failure >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< any >( __LINE__, __FILE__, "" );
      verify_required_rewind< eof >( __LINE__, __FILE__, "x" );
      verify_required_rewind< bof >( __LINE__, __FILE__, "xy", 1 );
      verify_required_rewind< bol >( __LINE__, __FILE__, "xy", 1 );
      verify_required_rewind< eol >( __LINE__, __FILE__, "x" );
      verify_required_rewind< eolf >( __LINE__, __FILE__, "x" );
      verify_required_rewind< consume< 3 > >( __LINE__, __FILE__, "xy" );

      verify_required_rewind< one< 'a' > >( __LINE__, __FILE__, "b" );
      verify_required_rewind< not_one< 'a' > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< range< 'a', 'z' > >( __LINE__, __FILE__, "A" );
      verify_required_rewind< not_range< 'a', 'z' > >( __LINE__, __FILE__, "m" );
      verify_required_rewind< ranges< 'a', 'f', '0', '3' > >( __LINE__, __FILE__, "x" );
      verify_required_rewind< not_ranges< 'a', 'f', '0', '3' > >( __LINE__, __FILE__, "c" );
      verify_required_rewind< string< "abc" > >( __LINE__, __FILE__, "abx" );
      verify_required_rewind< istring< "abc" > >( __LINE__, __FILE__, "abx" );

      verify_required_rewind< uint16_be::string< 0x0102, 0x0304 > >( __LINE__, __FILE__, "\x01\x02\x03" );
      verify_required_rewind< utf8::string< 0x20, 0x20ac > >( __LINE__, __FILE__, "\x20\xe2\x82" );
      verify_required_rewind< utf16_be::string< 0x20, 0x20ac > >( __LINE__, __FILE__, u16s_be( 0x20 ) + u16s_be( 0x20 ) );
      verify_required_rewind< utf16::string< 0x20, 0x20ac > >( __LINE__, __FILE__, u16s( 0x20 ) + u16s( 0x20 ) );

      verify_required_member_rewind< member::string< &member_token::value, 1, 2 > >( __LINE__, __FILE__, { { 1 }, { 3 } } );
   }

   void test_wrappers()
   {
      verify_required_rewind< apply< false_apply > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< apply0< false_apply0 > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< function< return_false > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< function< char_return_false, internal::peek_char > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< disable< seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< enable< seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< action< nothing, seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< control< normal, seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< state< test_state, seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
   }

   void test_predicates_and_choices()
   {
      verify_required_rewind< at< seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< not_at< one< 'a' > > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< minus< string< "ab" >, string< "ab" > > >( __LINE__, __FILE__, "ab" );

      verify_required_rewind< seq< one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< seq< one< 'a' >, any > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< sor< seq< one< 'a' >, one< 'b' > >, seq< one< 'a' >, one< 'c' > > > >( __LINE__, __FILE__, "ad" );
      verify_required_rewind< if_then_else< one< 'a' >, one< 'b' >, one< 'c' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< if_then_else< one< 'a' >, one< 'b' >, seq< one< 'c' >, one< 'd' > > > >( __LINE__, __FILE__, "cx" );
      verify_required_rewind< if_apply< string< "ab" >, false_apply > >( __LINE__, __FILE__, "ab" );
   }

   void test_repetition()
   {
      verify_required_rewind< plus< seq< one< 'a' >, one< 'b' > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< rep< 2, one< 'a' > > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< rep_max< 2, one< 'a' > > >( __LINE__, __FILE__, "aaa" );
      verify_required_rewind< rep_min< 2, one< 'a' > > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< rep_min_max< 2, 3, one< 'a' > > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< rep_min_max< 1, 2, one< 'a' > > >( __LINE__, __FILE__, "aaa" );

      verify_required_rewind< strict< one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< star_strict< one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< unordered< one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "a" );
      verify_required_rewind< until< one< 'z' > > >( __LINE__, __FILE__, "abc" );
      verify_required_rewind< until< one< 'z' >, one< 'a' > > >( __LINE__, __FILE__, "aaax" );
   }

   void test_structural_rules()
   {
      verify_required_rewind< list< seq< one< 'a' >, one< 'b' > >, one< ',' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< list_tail< seq< one< 'a' >, one< 'b' > >, one< ',' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< pad< one< 'a' >, one< ' ' > > >( __LINE__, __FILE__, "  x" );
      verify_required_rewind< separated< one< ',' >, one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "a,x" );
      verify_required_rewind< separated_pad< one< ',' >, one< ' ' >, one< 'a' >, one< 'b' > > >( __LINE__, __FILE__, "a , x" );
      verify_required_rewind< rematch< string< "ab" >, string< "ac" > > >( __LINE__, __FILE__, "ab" );
      verify_required_string_rewind< nested< source_inner, peek_string > >( __LINE__, __FILE__, { "bar" } );
      verify_required_source_rewind< source< source_inner > >( __LINE__, __FILE__, "bar", "abc" );
   }

#if defined( __cpp_exceptions )
   void test_exception_to_local_failure()
   {
      verify_required_rewind< if_must< seq< one< 'a' >, one< 'b' > >, one< 'c' > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< try_catch_return_false< seq< one< 'a' >, must< one< 'b' > > > > >( __LINE__, __FILE__, "ax" );
      verify_required_rewind< try_catch_any_return_false< seq< one< 'a' >, must< one< 'b' > > > > >( __LINE__, __FILE__, "ax" );
   }
#endif

   void unit_test()
   {
      test_atomic_rules();
      test_wrappers();
      test_predicates_and_choices();
      test_repetition();
      test_structural_rules();
#if defined( __cpp_exceptions )
      test_exception_to_local_failure();
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
