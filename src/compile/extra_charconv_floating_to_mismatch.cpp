// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl.hpp>
#include <tao/pegtl/extra/builders.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

struct target
{
   float value = 0;
};

#if TAO_PEGTL_COMPILE_ACCEPT
struct rule : pegtl::from_fchars_nothrow< float >
{};
#else
// A typed rule must agree with the builder's output type.
struct rule : pegtl::from_fchars_nothrow< double >
{};
#endif

template< typename Rule >
struct action : pegtl::nothing< Rule >
{};

template<>
struct action< rule > : pegtl::from_chars_to< &target::value >
{};

int main()
{
   target out;
   pegtl::view_input in( "1.25" );
   return pegtl::parse< rule, action >( in, out ) ? 0 : 1;
}
