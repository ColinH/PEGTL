// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>

#include "test.hpp"
#include "verify_file.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   using untracked_input = internal::view_input< char >;
   using tracked_input = internal::input_with_lines< ascii::scan::lf, untracked_input >;

   struct input_with_eol_data_member
      : untracked_input
   {
      static constexpr bool eol_rule = false;
   };

   template< typename Input >
   using rematched = internal::rematch_input< internal::rewind_guard< Input >, Input >;

   static_assert( std::same_as< rematched< untracked_input >, internal::rematch_input< internal::rewind_guard< untracked_input >, untracked_input, false > > );
   static_assert( std::same_as< rematched< input_with_eol_data_member >, internal::rematch_input< internal::rewind_guard< input_with_eol_data_member >, input_with_eol_data_member, false > > );
   static_assert( std::same_as< rematched< tracked_input >::eol_rule, tracked_input::eol_rule > );

   void unit_test()
   {
      internal::input_with_fakes< internal::read_input > in( "src/test/data/duseltronik.txt" );
      in.consume< any >( 1 );
      internal::rewind_guard rg( in );
      in.consume< any >( 4 );

      const auto end_position = in.rewind_position();
      const auto* const begin = rg.current();
      const auto* const end = in.current();

      internal::rematch_input ri( rg, in );
      TAO_PEGTL_TEST_ASSERT( ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 0 );
      TAO_PEGTL_TEST_ASSERT( ri.start() == begin );
      TAO_PEGTL_TEST_ASSERT( ri.current() == end );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );
      TAO_PEGTL_TEST_ASSERT( ri.end() == end );

      ri.restart();
      TAO_PEGTL_TEST_ASSERT( !ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 4 );
      TAO_PEGTL_TEST_ASSERT( ri.current() == begin );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );
      TAO_PEGTL_TEST_ASSERT( ri.end() == end );

      const auto begin_position = ri.rewind_position();
      ri.consume< any >( 1 );
      TAO_PEGTL_TEST_ASSERT( !ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 3 );
      TAO_PEGTL_TEST_ASSERT( ri.start() == begin );
      TAO_PEGTL_TEST_ASSERT( ri.current() == begin + 1 );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );
      TAO_PEGTL_TEST_ASSERT( ri.end() == end );
      TAO_PEGTL_TEST_ASSERT( ri.previous( begin_position ) == begin );

      ri.restart();
      TAO_PEGTL_TEST_ASSERT( !ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 4 );
      TAO_PEGTL_TEST_ASSERT( ri.current() == begin );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );
      TAO_PEGTL_TEST_ASSERT( ri.end() == end );

      ri.consume< any >( 2 );
      ri.rewind_to_position( begin_position );
      TAO_PEGTL_TEST_ASSERT( !ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 4 );
      TAO_PEGTL_TEST_ASSERT( ri.current() == begin );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );

      in.rewind_to_position( end_position );
      TAO_PEGTL_TEST_ASSERT( ri.empty() );
      TAO_PEGTL_TEST_ASSERT( ri.size() == 0 );
      TAO_PEGTL_TEST_ASSERT( ri.current() == end );
      TAO_PEGTL_TEST_ASSERT( ri.current() == in.current() );

      ri.restart();
      ri.consume< any >( 1 );
      const auto p1 = ri.current_position();
      TAO_PEGTL_TEST_ASSERT( p1.count == 2 );
      const auto p2 = ri.previous_position( begin_position );
      TAO_PEGTL_TEST_ASSERT( p2.count == 1 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
