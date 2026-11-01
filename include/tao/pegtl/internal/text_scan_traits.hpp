// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_TEXT_SCAN_TRAITS_HPP
#define TAO_PEGTL_INTERNAL_TEXT_SCAN_TRAITS_HPP

#include <type_traits>

#include "../config.hpp"
#include "../eol_exclude_tag.hpp"
#include "../eol_matched_tag.hpp"
#include "../eol_unknown_tag.hpp"

#include "ascii_istring.hpp"
#include "ascii_string.hpp"
#include "ascii_utility.hpp"
#include "at.hpp"
#include "eol.hpp"
#include "eolf.hpp"
#include "get_eol_rule_char.hpp"
#include "ione.hpp"
#include "lazy_scan_traits.hpp"
#include "not_ione.hpp"
#include "not_one.hpp"
#include "not_range.hpp"
#include "not_ranges.hpp"
#include "one.hpp"
#include "range.hpp"
#include "ranges.hpp"
#include "scan_base_classes.hpp"
#include "scan_input.hpp"
#include "until.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Eol, typename Rule >
   struct text_scan_traits;

   template< typename Eol >
   struct text_scan_traits< Eol, eol_exclude_tag >
      : add_column_scan
   {};

   template< typename Eol >
   struct text_scan_traits< Eol, eol_matched_tag >
      : nop_scan
   {};

   template< typename Eol >
   struct text_scan_traits< Eol, eol_unknown_tag >
      : lazy_scan_traits< typename Eol::eol_char_rule >
   {};

   template< typename Eol, typename TR >
   struct terminal_rule_traits
   {
      template< typename Position, typename Data >
      static void scan( Position& pos, scan_input< Data >& in )
      {
         if constexpr( std::is_same_v< typename Eol::eol_char_rule::rule_t, typename TR::rule_t > ) {
            inc_line_scan::scan( pos, in );
         }
         else if constexpr( TR::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) ) {
            lazy_scan_traits< typename Eol::eol_char_rule >::scan( pos, in );
         }
         else {
            add_column_scan::scan( pos, in );
         }
      }
   };

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, ione< Peek, Cs... > >
      : terminal_rule_traits< Eol, ione< Peek, Cs... > >
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, not_ione< Peek, Cs... > >
      : terminal_rule_traits< Eol, not_ione< Peek, Cs... > >
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, not_one< Peek, Cs... > >
      : terminal_rule_traits< Eol, not_one< Peek, Cs... > >
   {};

   template< typename Eol, typename Peek, Peek::data_t Lo, Peek::data_t Hi >
   struct text_scan_traits< Eol, not_range< Peek, Lo, Hi > >
      : terminal_rule_traits< Eol, not_range< Peek, Lo, Hi > >
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, not_ranges< Peek, Cs... > >
      : terminal_rule_traits< Eol, not_ranges< Peek, Cs... > >
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, one< Peek, Cs... > >
      : terminal_rule_traits< Eol, one< Peek, Cs... > >
   {};

   template< typename Eol, typename Peek, Peek::data_t Lo, Peek::data_t Hi >
   struct text_scan_traits< Eol, range< Peek, Lo, Hi > >
      : terminal_rule_traits< Eol, range< Peek, Lo, Hi > >
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
   struct text_scan_traits< Eol, ranges< Peek, Cs... > >
      : terminal_rule_traits< Eol, ranges< Peek, Cs... > >
   {};

   template< typename Eol, char... Cs >
      requires( ( !ascii_char_equal< Cs >( get_eol_rule_char_v< typename Eol::eol_char_rule > ) && ... ) )
   struct text_scan_traits< Eol, ascii_string< Cs... > >
      : add_column_scan
   {};

   template< typename Eol, char... Cs >
      requires( ( !ascii_ichar_equal< Cs >( get_eol_rule_char_v< typename Eol::eol_char_rule > ) && ... ) )
   struct text_scan_traits< Eol, ascii_istring< Cs... > >
      : add_column_scan
   {};

   template< typename Eol >
   struct text_scan_traits< Eol, until< eol > >
      : add_column_scan
   {};

   template< typename Eol >
   struct text_scan_traits< Eol, until< eolf > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( ione< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< ione< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( not_ione< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< not_ione< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( not_one< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< not_one< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t Lo, Peek::data_t Hi >
      requires( not_range< Peek, Lo, Hi >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< not_range< Peek, Lo, Hi > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( not_ranges< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< not_ranges< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( one< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< one< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t Lo, Peek::data_t Hi >
      requires( range< Peek, Lo, Hi >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< range< Peek, Lo, Hi > > >
      : add_column_scan
   {};

   template< typename Eol, typename Peek, Peek::data_t... Cs >
      requires( ranges< Peek, Cs... >::test( get_eol_rule_char_v< typename Eol::eol_char_rule > ) )
   struct text_scan_traits< Eol, until< ranges< Peek, Cs... > > >
      : add_column_scan
   {};

   template< typename Eol, typename Cond >
   struct text_scan_traits< Eol, until< at< Cond > > >
      : text_scan_traits< Eol, until< typename Cond::rule_t > >
   {};

   template< typename Eol, typename Cond >
      requires( !std::is_same_v< Cond, typename Cond::rule_t > )
   struct text_scan_traits< Eol, until< Cond > >
      : text_scan_traits< Eol, until< typename Cond::rule_t > >
   {};

   template< typename Eol, typename Rule >
   struct text_scan_traits
      : lazy_scan_traits< typename Eol::eol_char_rule >
   {
      static_assert( std::is_same_v< Rule, typename Rule::rule_t > );
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
