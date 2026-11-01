// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/unicode/utf16.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct fake_char_peek
   {
      using data_t = char;
      using pair_t = internal::char_and_size;

      template< typename >
      static constexpr bool bulk() noexcept
      {
         return true;
      }

      template< typename >
      static constexpr std::size_t size() noexcept
      {
         return 1;
      }
   };

   struct fake_lf
      : internal::one< fake_char_peek, '\n' >
   {};

   struct self_lazy_lf
      : ascii::lazy::lf
   {
      using rule_t = self_lazy_lf;
   };

   std::size_t ui = 0;
   std::array< std::size_t, 5 > ua = { 1, 1, 1, 1, 2 };

   struct test_utf16_peek
      : internal::peek_utf16
   {
      template< typename Input >
      [[nodiscard]] static auto peek( Input& in, std::size_t offset = 0 )
      {
         const auto pair = internal::peek_utf16::peek( in, offset );
         TAO_PEGTL_TEST_ASSERT( ui < ua.size() );
         TAO_PEGTL_TEST_ASSERT( pair.size() == ua[ ui++ ] );
         return pair;
      }
   };

   struct utf16_lf
      : internal::one< test_utf16_peek, '\n' >
   {};

   void unit_test()
   {
      const std::string s = "123\n456\n7890";
      {
         text_position pos;
         internal::scan_input< char > in( s );
         internal::lazy_scan_traits< ascii::lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 5 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 12 );
      }
      {
         text_position pos;
         internal::scan_input< char > in( s );
         internal::lazy_scan_traits< ascii::scan::lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 5 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 12 );
      }
      {
         text_position pos;
         internal::scan_input< char > in( s );
         internal::lazy_scan_traits< ascii::lazy::lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 5 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 12 );
      }
      {
         text_position pos;
         internal::scan_input< char > in( s );
         internal::lazy_scan_traits< self_lazy_lf >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 5 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 12 );
      }
      {
         text_position pos;
         internal::scan_input< char > in( s );
         internal::lazy_scan_traits< fake_lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 5 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 12 );
      }
      std::vector< std::uint16_t > v = { '1', '2', '3', '\n', 0xdbaa, 0xdfaa };
      {
         text_position pos;
         internal::scan_input< std::uint16_t > in( v );
         internal::lazy_scan_traits< utf16::lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 2 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 6 );
      }
      {
         text_position pos;
         internal::scan_input< std::uint16_t > in( v );
         internal::lazy_scan_traits< utf16_lf, void >::scan( pos, in );
         TAO_PEGTL_TEST_ASSERT( pos.line == 2 );
         TAO_PEGTL_TEST_ASSERT( pos.column == 3 );
         TAO_PEGTL_TEST_ASSERT( pos.count == 6 );
         TAO_PEGTL_TEST_ASSERT( ui == 5 );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
