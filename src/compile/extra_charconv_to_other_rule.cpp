// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl.hpp>

#include <tao/pegtl/extra/builders.hpp>
#include <tao/pegtl/extra/charconv.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

struct target
{
   int value = 0;
};

#if TAO_PEGTL_COMPILE_ACCEPT
struct rule
   : pegtl::from_chars_nothrow< int >
{};
#else
struct rule
   : pegtl::string< "42" >
{};
#endif

template< typename Rule >
struct action
   : pegtl::nothing< Rule >
{};

template<>
struct action< rule >
   : pegtl::from_chars_to< &target::value >
{};

int main()
{
#if TAO_PEGTL_COMPILE_REJECT
   // include/tao/pegtl/extra/internal/builder_utility.hpp
   // static_assert( is_from_chars_or_void_v< Integer, Rule > );
#endif

   target out;
   pegtl::view_input in( "42" );
   return pegtl::parse< rule, action >( in, out ) ? 0 : 1;
}
