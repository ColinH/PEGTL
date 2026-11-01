// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_CONTROL_INTERNAL_SHUFFLE_STATES_HPP
#define TAO_PEGTL_CONTROL_INTERNAL_SHUFFLE_STATES_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../../apply_mode.hpp"
#include "../../config.hpp"
#include "../../rewind_mode.hpp"

#include "../../internal/has_unwind.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Shuffle, typename Base >
   struct shuffle_states
      : Base
   {
      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void start_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::start( in, std::get< Shuffle::template value< Is, sizeof...( Is ) > >( t )... ) ) )
      {
         Base::start( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void start( const ParseInput& in, States&&... st ) noexcept( noexcept( start_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ) ) )
      {
         start_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename ParseInput, typename State >
      // static void start( const ParseInput& in, State&& st ) noexcept( noexcept( Base::start( in, st ) ) )
      // {
      //    Base::start( in, st );
      // }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void success_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::success( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ) ) )
      {
         Base::success( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void success( const ParseInput& in, States&&... st ) noexcept( noexcept( success_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ) ) )
      {
         success_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename ParseInput, typename State >
      // static void success( const ParseInput& in, State&& st ) noexcept( noexcept( Base::success( in, st ) ) )
      // {
      //    Base::success( in, st );
      // }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void failure_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::failure( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ) ) )
      {
         Base::failure( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void failure( const ParseInput& in, States&&... st ) noexcept( noexcept( failure_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ) ) )
      {
         failure_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename ParseInput, typename State >
      // static void failure( const ParseInput& in, State&& st ) noexcept( noexcept( Base::failure( in, st ) ) )
      // {
      //    Base::failure( in, st );
      // }

      template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control, typename ParseInput, typename Tuple, std::size_t... Is >
      static auto guard_impl( ParseInput&& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
      {
         return Base::template guard< A, M, Action, Control >( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control, typename ParseInput, typename... States >
      static auto guard( ParseInput&& in, States&&... st )
      {
         return guard_impl< A, M, Action, Control >( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control, typename ParseInput, typename State >
      // static auto guard( ParseInput&& in, State&& st )
      // {
      //    return Base::template guard< A, M, Action, Control >( in, st );
      // }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      [[noreturn]] static void raise_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
      {
         Base::raise( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename ParseInput, typename... States >
      [[noreturn]] static void raise( const ParseInput& in, States&&... st )
      {
         raise_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename ParseInput, typename State >
      // [[noreturn]] static void raise( const ParseInput& in, State&& st )
      // {
      //    Base::raise( in, st );
      // }

      template< typename Ambient, typename ParseInput, typename Tuple, std::size_t... Is >
      [[noreturn]] static void raise_nested_impl( const Ambient& am, const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
      {
         Base::raise_nested( am, in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename Ambient, typename ParseInput, typename... States >
      [[noreturn]] static void raise_nested( const Ambient& am, const ParseInput& in, States&&... st )
      {
         raise_nested_impl( am, in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename Ambient, typename ParseInput, typename State >
      // [[noreturn]] static void raise_nested( const Ambient& am, const ParseInput& in, State&& st )
      // {
      //    Base::raise_nested( am, in, st );
      // }

      template< typename ParseInput, typename Tuple, std::size_t... Is >
      static void unwind_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ )
         requires internal::has_unwind< Base, void, const ParseInput&, std::tuple_element_t< Shuffle::template value< Is, std::tuple_size_v< Tuple > >, Tuple >... >
      {
         Base::unwind( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< typename ParseInput, typename... States >
      static void unwind( const ParseInput& in, States&&... st )
         requires requires { unwind_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ); }
      {
         unwind_impl( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< typename ParseInput, typename State >
      // static void unwind( const ParseInput& in, State&& st )
      //    requires internal::has_unwind< Base, void, const ParseInput&, State >
      // {
      //    Base::unwind( in, st );
      // }

      template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename Tuple, std::size_t... Is >
      static decltype( auto ) apply_impl( const RewindPosition& begin, const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ) ) )
         requires requires { Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ); }
      {
         return Base::template apply< Action >( begin, in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ) ) )
         requires requires { apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ); }
      {
         return apply_impl< Action >( begin, in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename State >
      // static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, State&& st ) noexcept( noexcept( Base::template apply< Action >( begin, in, st ) ) )
      //    requires requires { Base::template apply< Action >( begin, in, st ); }
      // {
      //    return Base::template apply< Action >( begin, in, st );
      // }

      template< template< typename... > class Action, typename ParseInput, typename Tuple, std::size_t... Is >
      static decltype( auto ) apply0_impl( const ParseInput& in, const Tuple& t, std::index_sequence< Is... > /*unused*/ ) noexcept( noexcept( Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ) ) )
         requires requires { Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... ); }
      {
         return Base::template apply0< Action >( in, std::get< Shuffle::template value< Is, std::tuple_size_v< Tuple > > >( t )... );
      }

      template< template< typename... > class Action, typename ParseInput, typename... States >
      static decltype( auto ) apply0( const ParseInput& in, States&&... st ) noexcept( noexcept( apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ) ) )
         requires requires { apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() ); }
      {
         return apply0_impl< Action >( in, std::tie( st... ), std::make_index_sequence< Shuffle::template count< sizeof...( st ) > >() );
      }

      // template< template< typename... > class Action, typename ParseInput, typename State >
      // static decltype( auto ) apply0( const ParseInput& in, State&& st ) noexcept( noexcept( Base::template apply0< Action >( in, st ) ) )
      //    requires requires { Base::template apply0< Action >( in, st ); }
      // {
      //    return Base::template apply0< Action >( in, st );
      // }

      // Keep inherited match() to prevent a shuffle for every nested rule.
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
