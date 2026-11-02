// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "verify_ctrl.hpp"
#include "verify_meta.hpp"
#include "verify_rule.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   void unit_test()
   {
      verify_meta< restart, internal::restart >();

      verify_ctrl_enabled< restart >( __LINE__, __FILE__, "abc" );
      verify_ctrl_disabled< internal::restart >( __LINE__, __FILE__, "abc" );

      verify_rule< restart >( __LINE__, __FILE__, "", result_type::success, 0 );
      verify_rule< restart >( __LINE__, __FILE__, "abc", result_type::success, 3 );

      verify_rule< seq< one< 'a' >, restart > >( __LINE__, __FILE__, "a", result_type::success, 1 );
      verify_rule< seq< one< 'a' >, restart > >( __LINE__, __FILE__, "ab", result_type::success, 2 );

      verify_rule< seq< one< 'a' >, restart, one< 'a' > > >( __LINE__, __FILE__, "a", result_type::success, 0 );
      verify_rule< seq< one< 'a' >, restart, string< "ab" > > >( __LINE__, __FILE__, "ab", result_type::success, 0 );
      verify_rule< seq< one< 'a' >, restart, one< 'b' > > >( __LINE__, __FILE__, "ab", result_type::local_failure );
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
