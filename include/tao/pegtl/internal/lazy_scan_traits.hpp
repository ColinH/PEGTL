// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_LAZY_SCAN_TRAITS_HPP
#define TAO_PEGTL_INTERNAL_LAZY_SCAN_TRAITS_HPP

#include <type_traits>

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../normal.hpp"
#include "../nothing.hpp"
#include "../rewind_mode.hpp"

#include "has_eol_lazy_peek.hpp"
#include "is_simple_type.hpp"
#include "one.hpp"
#include "scan_input.hpp"
#include "test_after_scan.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Eol, typename = void >
   struct lazy_scan_traits;

   template< typename Eol, typename Peek >
      requires( !std::is_same_v< void, Peek > )
   struct lazy_scan_traits< Eol, Peek >
   {
      template< typename Position, typename Data >
      static void scan( Position& pos, scan_input< Data >& in )
      {
         pos.count += in.size();

         static_assert( is_simple_type_v< Data > );

         while( !in.empty() ) {
            if( normal< Eol >::template match< apply_mode::disabled, rewind_mode::required, nothing, normal >( in ) ) {
               pos.line++;
               pos.column = 1;
               continue;
            }
            if( const auto pair = Peek::peek( in ) ) {
               pos.column += pair.size();
               in.template consume< void >( pair.size() );
               continue;
            }
            break;
         }
         test_after_scan( in );
      }
   };

   template< auto Eol, typename Peek >
   struct lazy_scan_traits< one< Peek, Eol > >
   {
      template< typename Position, typename Data >
      static void scan( Position& pos, scan_input< Data >& in )
      {
         pos.count += in.size();

         using Char = decltype( Eol );

         static_assert( is_simple_type_v< Data > );

         constexpr bool same = ( sizeof( Char ) == sizeof( Data ) );
         constexpr bool bulk = Peek::template bulk< scan_input< Data > >();
         constexpr bool size = ( Peek::template size< scan_input< Data > >() == 1 );

         if constexpr( same && bulk && size ) {
            while( !in.empty() ) {
               if( in.template peek_as< Char >() == Eol ) {
                  pos.line++;
                  pos.column = 1;
               }
               else {
                  pos.column++;
               }
               in.template consume< void >( 1 );
            }
         }
         else {
            while( !in.empty() ) {
               if( const auto pair = Peek::peek( in ) ) {
                  if( one< Peek, Eol >::test( pair.data() ) ) {
                     pos.line++;
                     pos.column = 1;
                  }
                  else {
                     pos.column += pair.size();
                  }
                  in.template consume< void >( pair.size() );
                  continue;
               }
               break;
            }
         }
         test_after_scan( in );
      }
   };

   template< typename Eol >
      requires( !std::is_same_v< Eol, typename Eol::rule_t > && !has_eol_lazy_peek< Eol > )
   struct lazy_scan_traits< Eol >
      : lazy_scan_traits< typename Eol::rule_t, void >
   {};

   template< typename Eol >
      requires has_eol_lazy_peek< Eol >
   struct lazy_scan_traits< Eol >
      : lazy_scan_traits< typename Eol::rule_t, typename Eol::eol_lazy_peek >
   {};

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
