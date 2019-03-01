// Copyright (c) 2019 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_PEGTL_MATCH_HPP
#define TAO_PEGTL_MATCH_HPP

#include <type_traits>
#include <utility>

#include "apply_mode.hpp"
#include "config.hpp"
#include "require_apply.hpp"
#include "require_apply0.hpp"
#include "rewind_mode.hpp"

#include "internal/dusel_mode.hpp"
#include "internal/duseltronik.hpp"
#include "internal/has_apply.hpp"
#include "internal/has_apply0.hpp"
#include "internal/skip_control.hpp"

namespace tao
{
   namespace TAO_PEGTL_NAMESPACE
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... >
                class Action,
                template< typename... >
                class Control,
                typename Input,
                typename... States >
      [[nodiscard]] bool match( Input& in, States&&... st )
      {
         constexpr bool use_control = !internal::skip_control< Rule >;
         constexpr bool use_apply = use_control && ( A == apply_mode::action );

         constexpr bool has_apply_void = use_apply && internal::has_apply< Control< Rule >, void, Action, const typename Input::iterator_t&, const Input&, States... >::value;
         constexpr bool has_apply_bool = use_apply && internal::has_apply< Control< Rule >, bool, Action, const typename Input::iterator_t&, const Input&, States... >::value;
         constexpr bool has_apply = has_apply_void || has_apply_bool;

         constexpr bool has_apply0_void = use_apply && internal::has_apply0< Control< Rule >, void, Action, const Input&, States... >::value;
         constexpr bool has_apply0_bool = use_apply && internal::has_apply0< Control< Rule >, bool, Action, const Input&, States... >::value;
         constexpr bool has_apply0 = has_apply0_void || has_apply0_bool;

         constexpr bool is_required_apply0 = std::is_base_of_v< require_apply0, Action< Rule > >;
         constexpr bool is_required_apply = std::is_base_of_v< require_apply, Action< Rule > >;

         static_assert( !is_required_apply || has_apply, "required_apply did not have apply()" );
         static_assert( !is_required_apply0 || has_apply0, "required_apply0 did not have apply0()" );
         static_assert( !( has_apply && has_apply0 ), "both apply() and apply0() defined" );

         constexpr auto mode = static_cast< dusel_mode >( use_control + has_apply_void + 2 * has_apply_bool + 3 * has_apply0_void + 4 * has_apply0_bool );
         return internal::duseltronik< Rule, A, M, Action, Control, mode >::match( in, st... );
      }

   }  // namespace TAO_PEGTL_NAMESPACE

}  // namespace tao

#endif