// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>

#include "test.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Input >
   concept has_direct_source = requires( const Input& in ) { in.direct_source(); };

   template< typename Input >
   concept has_direct_position = requires( const Input& in ) { in.direct_position(); };

   using sourced_input = view_input< scan::lf, char, std::string >;
   using tracked_input = text_view_input< scan::lf >;
   using sourced_tracked_input = text_view_input< scan::lf, char, std::string >;

   struct mutable_metadata
      : view_input< scan::lf >
   {
      std::string& direct_source() &;
      count_position& direct_position() &;
   };

   static_assert( !has_direct_source< action_input< view_input< scan::lf > > > );
   static_assert( !has_direct_position< action_input< view_input< scan::lf > > > );
   static_assert( has_direct_source< action_input< sourced_input > > );
   static_assert( !has_direct_position< action_input< sourced_input > > );
   static_assert( !has_direct_source< action_input< tracked_input > > );
   static_assert( has_direct_position< action_input< tracked_input > > );
   static_assert( has_direct_source< action_input< sourced_tracked_input > > );
   static_assert( has_direct_position< action_input< sourced_tracked_input > > );
   static_assert( !has_direct_position< action_input< input_with_offset< tracked_input > > > );
   static_assert( !has_direct_source< action_input< mutable_metadata > > );
   static_assert( !has_direct_position< action_input< mutable_metadata > > );

   void unit_test()
   {
      view_input< scan::lf > in( "hallo" );
      in.consume< eol_exclude_tag >( 1 );
      {
         const auto p = in.rewind_position();
         in.consume< eol_exclude_tag >( 3 );
         action_input< view_input< scan::lf > > ai( p, in );
         TAO_PEGTL_TEST_ASSERT( ai.begin() == ai.current() );
         TAO_PEGTL_TEST_ASSERT( ai.begin() + 2 == ai.current( 2 ) );
         TAO_PEGTL_TEST_ASSERT( ai.begin() + 3 == ai.end() );
         TAO_PEGTL_TEST_ASSERT( !ai.empty() );
         TAO_PEGTL_TEST_ASSERT( ai.size() == 3 );
         TAO_PEGTL_TEST_ASSERT( &ai.input() == &in );
         TAO_PEGTL_TEST_ASSERT( ai.current_position() == in.previous_position( p ) );
         TAO_PEGTL_TEST_ASSERT( ai.rewind_position() == p );
         TAO_PEGTL_TEST_ASSERT( ai.begin() + 3 == in.current() );
      }
      {
         const auto p = in.rewind_position();
         action_input< view_input< scan::lf > > ai( p, in );
         TAO_PEGTL_TEST_ASSERT( ai.empty() );
         TAO_PEGTL_TEST_ASSERT( ai.size() == 0 );
      }
      {
         sourced_tracked_input tracked( "source", "abc" );
         const auto p = tracked.rewind_position();
         tracked.consume< eol_exclude_tag >( 2 );
         const action_input< sourced_tracked_input > ai( p, tracked );
         static_assert( std::same_as< decltype( ai.direct_source() ), const std::string& > );
         static_assert( std::same_as< decltype( ai.direct_position() ), const sourced_tracked_input::error_position_t& > );
         TAO_PEGTL_TEST_ASSERT( &ai.direct_source() == &tracked.direct_source() );
         TAO_PEGTL_TEST_ASSERT( &ai.direct_position() == &tracked.direct_position() );
         TAO_PEGTL_TEST_ASSERT( ai.direct_position().count == 2 );
         TAO_PEGTL_TEST_ASSERT( ai.current_position().count == 0 );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
