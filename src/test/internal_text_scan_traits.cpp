// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <string>

#include "test.hpp"
#include "test_utility.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Eol, typename Rule >
   void scan_test( const std::string& str, const std::size_t line, const std::size_t column, const std::size_t count )
   {
      text_position pos;
      internal::scan_input< char > in( str.data(), str.size() );
      internal::text_scan_traits< Eol, typename Rule::rule_t >::scan( pos, in );
      TAO_PEGTL_TEST_ASSERT( test::equal( pos, line, column, count ) );
   }

   void unit_test()
   {
      scan_test< scan::lf, eol_exclude_tag >( "", 1, 1, 0 );
      scan_test< scan::lf, eol_exclude_tag >( " ", 1, 2, 1 );
      scan_test< scan::lf, eol_exclude_tag >( "\n", 1, 2, 1 );

      scan_test< scan::lf, eol_matched_tag >( "", 1, 1, 0 );
      scan_test< scan::lf, eol_matched_tag >( " ", 1, 1, 0 );
      scan_test< scan::lf, eol_matched_tag >( "\n", 1, 1, 0 );

      scan_test< scan::lf, eol_unknown_tag >( "", 1, 1, 0 );
      scan_test< scan::lf, eol_unknown_tag >( " ", 1, 2, 1 );
      scan_test< scan::lf, eol_unknown_tag >( "     ", 1, 6, 5 );
      scan_test< scan::lf, eol_unknown_tag >( "\n", 2, 1, 1 );
      scan_test< scan::lf, eol_unknown_tag >( " \n", 2, 1, 2 );
      scan_test< scan::lf, eol_unknown_tag >( "   \n", 2, 1, 4 );
      scan_test< scan::lf, eol_unknown_tag >( "\n ", 2, 2, 2 );
      scan_test< scan::lf, eol_unknown_tag >( "   \n ", 2, 2, 5 );
      scan_test< scan::lf, eol_unknown_tag >( "   \n   ", 2, 4, 7 );
      scan_test< scan::lf, eol_unknown_tag >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf, eol_unknown_tag >( "\n \n", 3, 1, 3 );
      scan_test< scan::lf, eol_unknown_tag >( " \n \n", 3, 1, 4 );
      scan_test< scan::lf, eol_unknown_tag >( " \n \n ", 3, 2, 5 );
      scan_test< scan::lf, eol_unknown_tag >( "   \n\n\n   \n\n\n", 7, 1, 12 );
      scan_test< scan::lf, eol_unknown_tag >( "   \n\n\n   \n\n\n   ", 7, 4, 15 );

      scan_test< scan::lf, ascii::lf >( "", 2, 1, 0 );
      scan_test< scan::lf, ascii::lf >( " ", 2, 1, 1 );
      scan_test< scan::lf, ascii::lf >( "\n", 2, 1, 1 );
      scan_test< scan::lf, ascii::lf >( "\n\n", 2, 1, 2 );
      scan_test< scan::lf, ascii::lf >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf, ascii::space >( "", 1, 1, 0 );
      scan_test< scan::lf, ascii::space >( " ", 1, 2, 1 );
      scan_test< scan::lf, ascii::space >( "\n", 2, 1, 1 );
      scan_test< scan::lf, ascii::space >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf, ascii::space >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf, ascii::digit >( "", 1, 1, 0 );
      scan_test< scan::lf, ascii::digit >( " ", 1, 2, 1 );
      scan_test< scan::lf, ascii::digit >( "\n", 1, 2, 1 );
      scan_test< scan::lf, ascii::digit >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, ascii::digit >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< eol > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< eol > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< eol > >( "\n", 1, 2, 1 );
      scan_test< scan::lf, until< eol > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, until< eol > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< at< eol > > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< at< eol > > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< at< eol > > >( "\n", 1, 2, 1 );
      scan_test< scan::lf, until< at< eol > > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, until< at< eol > > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< eolf > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< eolf > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< eolf > >( "\n", 1, 2, 1 );
      scan_test< scan::lf, until< eolf > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, until< eolf > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< at< eolf > > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< at< eolf > > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< at< eolf > > >( "\n", 1, 2, 1 );
      scan_test< scan::lf, until< at< eolf > > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, until< at< eolf > > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< ascii::lf > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< ascii::lf > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< ascii::lf > >( "\n", 1, 2, 1 );
      scan_test< scan::lf, until< ascii::lf > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf, until< ascii::lf > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf, until< ascii::digit > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< ascii::digit > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< ascii::digit > >( "\n", 2, 1, 1 );
      scan_test< scan::lf, until< ascii::digit > >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf, until< ascii::digit > >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf, until< at< ascii::digit > > >( "", 1, 1, 0 );
      scan_test< scan::lf, until< at< ascii::digit > > >( " ", 1, 2, 1 );
      scan_test< scan::lf, until< at< ascii::digit > > >( "\n", 2, 1, 1 );
      scan_test< scan::lf, until< at< ascii::digit > > >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf, until< at< ascii::digit > > >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf, ascii::string< "abc" > >( "\n\n\n", 1, 4, 3 );
      scan_test< scan::lf, ascii::string< "\n\n\n" > >( "\n\n\n", 4, 1, 3 );
      scan_test< scan::lf, ascii::istring< "abc" > >( "\n\n\n", 1, 4, 3 );
      scan_test< scan::lf, ascii::istring< "\n\n\n" > >( "\n\n\n", 4, 1, 3 );
      scan_test< scan::lf, internal::ascii_string< 'a', 'b', 'c' > >( "\n\n\n", 1, 4, 3 );
      scan_test< scan::lf, internal::ascii_string< '\n', '\n', '\n' > >( "\n\n\n", 4, 1, 3 );

      scan_test< scan::lf_crlf, eol_exclude_tag >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_exclude_tag >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, eol_exclude_tag >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, eol_exclude_tag >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, eol_matched_tag >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_matched_tag >( " ", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_matched_tag >( "\n", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_matched_tag >( "\r\n", 1, 1, 0 );

      scan_test< scan::lf_crlf, eol_unknown_tag >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "     ", 1, 6, 5 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( " \n", 2, 1, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "   \n", 2, 1, 4 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n ", 2, 2, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "   \n ", 2, 2, 5 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "   \n   ", 2, 4, 7 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n \n", 3, 1, 3 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( " \n \n", 3, 1, 4 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( " \n \n ", 3, 2, 5 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "   \n\n\n   \n\n\n", 7, 1, 12 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "   \n\n\n   \n\n\n   ", 7, 4, 15 );

      scan_test< scan::lf_crlf, eol_unknown_tag >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "  ", 1, 3, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\r", 1, 2, 1 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\n\r", 2, 2, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "\r\n", 2, 1, 2 );
      scan_test< scan::lf_crlf, eol_unknown_tag >( "  \r\n  ", 2, 3, 6 );

      scan_test< scan::lf_crlf, ascii::lf >( "", 2, 1, 0 );
      scan_test< scan::lf_crlf, ascii::lf >( " ", 2, 1, 1 );
      scan_test< scan::lf_crlf, ascii::lf >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, ascii::lf >( "\n\n", 2, 1, 2 );
      scan_test< scan::lf_crlf, ascii::lf >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf_crlf, ascii::space >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, ascii::space >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, ascii::space >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, ascii::space >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf_crlf, ascii::space >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf_crlf, ascii::digit >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, ascii::digit >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, ascii::digit >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, ascii::digit >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, ascii::digit >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< eol > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< eol > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< eol > >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< eol > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, until< eol > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< at< eol > > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< at< eol > > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< at< eol > > >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< at< eol > > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, until< at< eol > > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< eolf > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< eolf > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< eolf > >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< eolf > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, until< eolf > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< at< eolf > > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< at< eolf > > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< at< eolf > > >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< at< eolf > > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, until< at< eolf > > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< ascii::lf > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< ascii::lf > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< ascii::lf > >( "\n", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< ascii::lf > >( "\n\n", 1, 3, 2 );
      scan_test< scan::lf_crlf, until< ascii::lf > >( "\r\n", 1, 3, 2 );

      scan_test< scan::lf_crlf, until< ascii::digit > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< ascii::digit > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< ascii::digit > >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, until< ascii::digit > >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf_crlf, until< ascii::digit > >( "\r\n", 2, 1, 2 );

      scan_test< scan::lf_crlf, until< at< ascii::digit > > >( "", 1, 1, 0 );
      scan_test< scan::lf_crlf, until< at< ascii::digit > > >( " ", 1, 2, 1 );
      scan_test< scan::lf_crlf, until< at< ascii::digit > > >( "\n", 2, 1, 1 );
      scan_test< scan::lf_crlf, until< at< ascii::digit > > >( "\n\n", 3, 1, 2 );
      scan_test< scan::lf_crlf, until< at< ascii::digit > > >( "\r\n", 2, 1, 2 );
      scan_test< scan::lf_crlf, ascii::string< "abc" > >( "\n\n\n", 1, 4, 3 );
      scan_test< scan::lf_crlf, ascii::istring< "abc" > >( "\n\n\n", 1, 4, 3 );
      scan_test< scan::lf_crlf, internal::ascii_string< 'a', 'b', 'c' > >( "\n\n\n", 1, 4, 3 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
