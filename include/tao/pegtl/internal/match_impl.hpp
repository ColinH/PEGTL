// Copyright (c) 2019-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_MATCH_IMPL_HPP
#define TAO_PEGTL_INTERNAL_MATCH_IMPL_HPP

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../rewind_mode.hpp"

#include "has_unwind.hpp"
#include "unwind_guard.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] static decltype( auto ) match_no_control( ParseInput& in, States&&... st )
      requires requires { Rule::template match< A, M, Action, Control >( in, st... ); }
   {
      return Rule::template match< A, M, Action, Control >( in, st... );
   }

   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] static decltype( auto ) match_no_control( ParseInput& in, States&&... /*unused*/ )
      requires requires { Rule::match( in ); }
   {
      return Rule::match( in );
   }

   template< typename Rule,
             apply_mode A,
             rewind_mode M,
             template< typename... > class Action,
             template< typename... > class Control,
             typename ParseInput,
             typename... States >
   [[nodiscard]] auto match_control_unwind( ParseInput& in, States&&... st )
   {
#if defined( __cpp_exceptions )
      if constexpr( has_unwind< Control< Rule >, void, const ParseInput&, States... > ) {
         unwind_guard ug( [ & ] {
            Control< Rule >::unwind( static_cast< const ParseInput& >( in ), st... );
         } );
         const auto result = match_no_control< Rule, A, M, Action, Control >( in, st... );
         ug.unwind.reset();
         return result;
      }
      else {
         return match_no_control< Rule, A, M, Action, Control >( in, st... );
      }
#else
      return match_no_control< Rule, A, M, Action, Control >( in, st... );
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
