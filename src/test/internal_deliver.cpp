// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "test.hpp"

#include <tao/pegtl/action/internal/deliver_utility.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   namespace delivery_test
   {
      struct target
      {
         int value = 0;
         std::unique_ptr< int > owned;

         void set( const int v )
         {
            value = v;
         }

         void set_nothrow( const int v ) noexcept
         {
            value = v;
         }

         void set_pair( const int v, std::unique_ptr< int > p ) noexcept
         {
            value = v;
            owned = std::move( p );
         }

         void reset() noexcept
         {
            value = 0;
         }
      };

      void set( target& out, const int& value )
      {
         out.value = value;
      }

      void set_nothrow( target& out, const int& value ) noexcept
      {
         out.value = value;
      }

      void set_pair( target& out, const int value, std::unique_ptr< int > p )
      {
         out.set_pair( value, std::move( p ) );
      }

      void reset( target& out ) noexcept
      {
         out.reset();
      }

      static_assert( std::is_same_v< internal::delivery_target_t< &target::set_nothrow >, target > );
      static_assert( std::is_same_v< internal::delivery_result_t< &target::set_nothrow >, int > );
      static_assert( std::is_same_v< internal::delivery_result_t< set_nothrow >, int > );
      static_assert( std::is_same_v< internal::delivery_result_t< &target::set_pair >, std::tuple< int, std::unique_ptr< int > > > );
      static_assert( std::is_same_v< internal::delivery_result_t< &target::reset >, std::tuple<> > );
      static_assert( std::is_same_v< internal::delivery_result_t< reset >, std::tuple<> > );
   }  // namespace delivery_test

   void unit_test()
   {
      using namespace delivery_test;
      target out;
      internal::deliver< &target::value >( out, 1 );
      TAO_PEGTL_TEST_ASSERT( out.value == 1 );
      internal::deliver< &target::owned >( out, std::make_unique< int >( 2 ) );
      TAO_PEGTL_TEST_ASSERT( *out.owned == 2 );

      internal::deliver< &target::set >( out, 3 );
      TAO_PEGTL_TEST_ASSERT( out.value == 3 );
      internal::deliver< &target::set_nothrow >( out, 4 );
      TAO_PEGTL_TEST_ASSERT( out.value == 4 );
      internal::deliver< set >( out, 5 );
      TAO_PEGTL_TEST_ASSERT( out.value == 5 );
      internal::deliver< set_nothrow >( out, 6 );
      TAO_PEGTL_TEST_ASSERT( out.value == 6 );

      internal::deliver< &target::set_pair >( out, std::make_tuple( 7, std::make_unique< int >( 8 ) ) );
      TAO_PEGTL_TEST_ASSERT( out.value == 7 && *out.owned == 8 );
      internal::deliver< set_pair >( out, std::make_tuple( 9, std::make_unique< int >( 10 ) ) );
      TAO_PEGTL_TEST_ASSERT( out.value == 9 && *out.owned == 10 );

      internal::deliver< &target::reset >( out, std::tuple<>() );
      TAO_PEGTL_TEST_ASSERT( out.value == 0 );
      out.value = 11;
      internal::deliver< reset >( out, std::tuple<>() );
      TAO_PEGTL_TEST_ASSERT( out.value == 0 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
