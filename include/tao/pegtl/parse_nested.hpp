// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_PARSE_NESTED_HPP
#define TAO_PEGTL_PARSE_NESTED_HPP

#include <type_traits>

#if defined( __cpp_exceptions )
#include <exception>
#endif

#include "apply_mode.hpp"
#include "config.hpp"
#include "normal.hpp"
#include "nothing.hpp"
#include "parse.hpp"
#include "rewind_mode.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   template< typename Rule,
             template< typename... > class Action = nothing,
             template< typename... > class Control = normal,
             apply_mode A = apply_mode::enabled,
             rewind_mode M = rewind_mode::optional,
             typename Ambient,
             typename ParseInput,
             typename... States >
   auto parse_nested( const Ambient& am, ParseInput&& in, States&&... st )
      requires( !std::is_const_v< std::remove_reference_t< ParseInput > > )
   {
#if defined( __cpp_exceptions )
      try {
         return parse< Rule, Action, Control, A, M >( in, st... );
      }
      catch( std::exception& /*unused*/ ) {
         Control< Rule >::raise_nested( am, in, st... );
      }
#else
      (void)am;
      return parse< Rule, Action, Control, A, M >( in, st... );
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#endif
