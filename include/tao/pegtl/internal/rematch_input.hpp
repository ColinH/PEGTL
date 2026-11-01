// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_REMATCH_INPUT_HPP
#define TAO_PEGTL_INTERNAL_REMATCH_INPUT_HPP

#include <cstddef>
#include <type_traits>

#include "../config.hpp"
#if defined( __cpp_exceptions )
#include "../parse_error.hpp"
#endif

#include "input_with_fakes.hpp"
#include "input_with_funcs.hpp"
#include "rewind_guard.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Guard, typename Input >
   class rematch_input_impl
   {
   public:
      using data_t = Input::data_t;
      using error_position_t = Input::error_position_t;
      using rewind_position_t = Input::rewind_position_t;
#if defined( __cpp_exceptions )
      using parse_error_t = parse_error< error_position_t >;
#endif

      rematch_input_impl( Guard& m, Input& in )
         : m_guard( m ),
           m_input( in ),
           m_end( in.current() )
      {}

      rematch_input_impl( rematch_input_impl&& ) = delete;
      rematch_input_impl( const rematch_input_impl& ) = delete;

      void operator=( rematch_input_impl&& ) = delete;
      void operator=( const rematch_input_impl& ) = delete;

      ~rematch_input_impl() = default;

      [[nodiscard]] bool empty() const noexcept
      {
         return current() == end();
      }

      [[nodiscard]] std::size_t size() const noexcept
      {
         return end() - current();
      }

      [[nodiscard]] const data_t* start() const noexcept
      {
         return m_guard.current();
      }

      void restart() noexcept
      {
         m_input.rewind_to_position( m_guard.rewind_position() );
      }

      [[nodiscard]] const data_t* current( const std::size_t offset = 0 ) const noexcept
      {
         return m_input.current( offset );
      }

      [[nodiscard]] const data_t* end() const noexcept
      {
         return m_end;
      }

      template< typename Position >
      [[nodiscard]] decltype( auto ) previous( const Position& saved ) const noexcept
      {
         return m_input.previous( saved );
      }

      template< typename Rule >
      void consume( const std::size_t count )
      {
         m_input.template consume< Rule >( count );
      }

      [[nodiscard]] decltype( auto ) rewind_position() const noexcept
      {
         return m_input.rewind_position();
      }

      void rewind_to_position( const rewind_position_t& saved ) noexcept
      {
         m_input.rewind_to_position( saved );
      }

      [[nodiscard]] decltype( auto ) current_position() const noexcept
      {
         return m_input.current_position();
      }

      template< typename Position >
      [[nodiscard]] decltype( auto ) previous_position( const Position& saved ) const noexcept
      {
         return m_input.previous_position( saved );
      }

      [[nodiscard]] decltype( auto ) direct_source() const noexcept
         requires requires( Input& in ) {
            in.direct_source();
         }
      {
         return m_input.direct_source();
      }

      [[nodiscard]] decltype( auto ) direct_position() const noexcept
         requires requires( Input& in ) {
            in.direct_position();
         }
      {
         return m_input.direct_position();
      }

      void private_set_end( const data_t* end ) noexcept
      {
         m_end = end;
      }

   protected:
      const Guard& m_guard;
      Input& m_input;
      const data_t* m_end;
   };

   template< typename Guard, typename Input, bool = requires { typename Input::eol_rule; } >
   struct rematch_input;

   template< typename Guard, typename Input >
   struct rematch_input< Guard, Input, false >
      : input_with_fakes< input_with_funcs< rematch_input_impl< Guard, Input > > >
   {
      using input_with_fakes< input_with_funcs< rematch_input_impl< Guard, Input > > >::input_with_fakes;
   };

   template< typename Guard, typename Input >
   struct rematch_input< Guard, Input, true >
      : rematch_input< Guard, Input, false >
   {
      using eol_rule = Input::eol_rule;

      using rematch_input< Guard, Input, false >::rematch_input;
   };

   template< typename Guard, typename Input >
   rematch_input( Guard&, Input& ) -> rematch_input< Guard, Input >;

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
