// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_CONTROL_INTERNAL_APPLY_TYPED_STATE_HPP
#define TAO_PEGTL_CONTROL_INTERNAL_APPLY_TYPED_STATE_HPP

#include <tuple>

#include "../../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Typed, typename Base >
   struct apply_typed_state
      : Base
   {
      template< template< typename... > class Action, typename RewindPosition, typename ParseInput, typename... States >
      static decltype( auto ) apply( const RewindPosition& begin, const ParseInput& in, States&&... st ) noexcept( noexcept( Base::template apply< Action >( begin, in, std::get< Typed& >( std::tie( st... ) ) ) ) )
         requires requires { Base::template apply< Action >( begin, in, std::get< Typed& >( std::tie( st... ) ) ); }
      {
         return Base::template apply< Action >( begin, in, std::get< Typed& >( std::tie( st... ) ) );
      }

      template< template< typename... > class Action, typename ParseInput, typename... States >
      static decltype( auto ) apply0( const ParseInput& in, States&&... st ) noexcept( noexcept( Base::template apply0< Action >( in, std::get< Typed& >( std::tie( st... ) ) ) ) )
         requires requires { Base::template apply0< Action >( in, std::get< Typed& >( std::tie( st... ) ) ); }
      {
         return Base::template apply0< Action >( in, std::get< Typed& >( std::tie( st... ) ) );
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
