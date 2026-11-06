// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/extra/builders.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

struct integer_target
{
   int value = 0;
};

struct bool_target
{
   bool value = false;
};

struct floating_target
{
   float value = 0;
   double other = 0;
};

#if TAO_PEGTL_COMPILE_ACCEPT
using action = pegtl::from_chars_to< &integer_target::value >;
#if defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
using float_action = pegtl::from_chars_to< &floating_target::value >;
using double_action = pegtl::from_chars_to< &floating_target::other >;
#endif
#else
// include/tao/pegtl/extra/builders.hpp
// from_chars_to only accepts supported integer and floating-point types.
using action = pegtl::from_chars_to< &bool_target::value >;
#endif

int main()
{
   ( void )sizeof( action );
#if TAO_PEGTL_COMPILE_ACCEPT && defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L
   ( void )sizeof( float_action );
   ( void )sizeof( double_action );
#endif
   return 0;
}
