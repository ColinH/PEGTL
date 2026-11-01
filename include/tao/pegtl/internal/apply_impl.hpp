// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_APPLY_IMPL_HPP
#define TAO_PEGTL_INTERNAL_APPLY_IMPL_HPP

#include <concepts>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Action >
   struct apply_impl
   {
      template< typename ActionInput, typename... States >
      [[nodiscard]] static bool apply( const ActionInput& in, States&&... st ) noexcept( noexcept( Action::apply( in, st... ) ) )
         requires requires { { Action::apply( in, st... ) } -> std::same_as< void >; }
      {
         Action::apply( in, st... );
         return true;
      }

      template< typename ActionInput, typename... States >
      [[nodiscard]] static bool apply( const ActionInput& in, States&&... st ) noexcept( noexcept( Action::apply( in, st... ) ) )
         requires requires { { Action::apply( in, st... ) } -> std::same_as< bool >; }
      {
         return Action::apply( in, st... );
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
