// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/extra/charconv.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

#if TAO_PEGTL_COMPILE_ACCEPT
using rule = pegtl::from_chars_nothrow< int >;
#else
// include/tao/pegtl/extra/internal/charconv.hpp
// static_assert( is_from_chars_type_v< Number, Format > );
using rule = pegtl::from_chars_nothrow< double >;
#endif

int main()
{
   (void)sizeof( rule );
   return 0;
}
