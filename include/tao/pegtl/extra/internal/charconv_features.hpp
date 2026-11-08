// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXTRA_INTERNAL_CHARCONV_FEATURES_HPP
#define TAO_PEGTL_EXTRA_INTERNAL_CHARCONV_FEATURES_HPP

#include <charconv>

// libc++ can provide the float and double overloads before defining __cpp_lib_to_chars.
// Its availability macro also accounts for the deployment target.
#if ( defined( __cpp_lib_to_chars ) && __cpp_lib_to_chars >= 201611L ) || ( defined( _LIBCPP_AVAILABILITY_HAS_FROM_CHARS_FLOATING_POINT ) && _LIBCPP_AVAILABILITY_HAS_FROM_CHARS_FLOATING_POINT )
#define TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS 1
#endif

#endif
