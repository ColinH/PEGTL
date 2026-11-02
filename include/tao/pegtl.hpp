// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_HPP
#define TAO_PEGTL_HPP

#include "pegtl/ascii.hpp"
#include "pegtl/demangle.hpp"
#include "pegtl/inputs.hpp"
#include "pegtl/parse.hpp"
#include "pegtl/rules.hpp"
#include "pegtl/version.hpp"

#if defined( __cpp_exceptions )
#include "pegtl/parse_error.hpp"
#include "pegtl/parse_error_base.hpp"
#include "pegtl/parse_nested.hpp"
#endif

#include "pegtl/unicode/utf8.hpp"

// FCOV_EXCL_FILE
#endif
