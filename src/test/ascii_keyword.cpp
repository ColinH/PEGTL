// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      verify_analyze< keyword< "foo" > >( __LINE__, __FILE__, true, false );

      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "foo", result_type::success, 0 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "foo ", result_type::success, 1 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "foo foo", result_type::success, 4 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "FOO", result_type::local_failure, 3 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "", result_type::local_failure, 0 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "f", result_type::local_failure, 1 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "fo", result_type::local_failure, 2 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, " foo", result_type::local_failure, 4 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "foo_", result_type::local_failure, 4 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "foo1", result_type::local_failure, 4 );
      verify_rule< keyword< "foo" > >( __LINE__, __FILE__, "fooa", result_type::local_failure, 4 );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
