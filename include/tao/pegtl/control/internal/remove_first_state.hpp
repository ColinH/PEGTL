// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_CONTROL_INTERNAL_REMOVE_FIRST_STATE_HPP
#define TAO_PEGTL_CONTROL_INTERNAL_REMOVE_FIRST_STATE_HPP

#include "../../apply_mode.hpp"
#include "../../config.hpp"
#include "../../rewind_mode.hpp"

#include "../../internal/has_unwind.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Base >
   struct remove_first_state
      : Base
   {
      template< typename ParseInput, typename State, typename... States >
      static void start( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::start( in, st... ) ) )
      {
         Base::start( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static void success( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::success( in, st... ) ) )
      {
         Base::success( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static void failure( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::failure( in, st... ) ) )
      {
         Base::failure( in, st... );
      }

      template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class ActiveControl, typename ParseInput, typename State, typename... States >
      static auto guard( ParseInput&& in, State&& /*unused*/, States&&... st )
      {
         return Base::template guard< A, M, Action, ActiveControl >( in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, State&& /*unused*/, States&&... st )
      {
         Base::raise( in, st... );
      }

      template< typename Ambient, typename ParseInput, typename State, typename... States >
      [[noreturn]] static void raise_nested( const Ambient& am, const ParseInput& in, State&& /*unused*/, States&&... st )
      {
         Base::raise_nested( am, in, st... );
      }

      template< typename ParseInput, typename State, typename... States >
      static void unwind( const ParseInput& in, State&& /*unused*/, States&&... st )
         requires internal::has_unwind< Base, void, const ParseInput&, States... >
      {
         Base::unwind( in, st... );
      }

      template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename State, typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::template apply< Action >( begin, in, st... ) ) )
         requires requires { Base::template apply< Action >( begin, in, st... ); }
      {
         return Base::template apply< Action >( begin, in, st... );
      }

      template< template< typename... > class Action, typename ParseInput, typename State, typename... States >
      static decltype( auto ) apply0( const ParseInput& in, State&& /*unused*/, States&&... st ) noexcept( noexcept( Base::template apply0< Action >( in, st... ) ) )
         requires requires { Base::template apply0< Action >( in, st... ); }
      {
         return Base::template apply0< Action >( in, st... );
      }

      // Keep inherited match() to prevent a first state being removed for every nested rule.
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
