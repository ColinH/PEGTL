// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#if !defined( __cpp_exceptions )
#include <iostream>
int main()
{
   std::cout << "Exception support disabled, skipping test..." << std::endl;
}
#else

#include "test.hpp"
#include "test_utility.hpp"

#include <tao/pegtl/control/must_if.hpp>

namespace test1
{
   using namespace TAO_PEGTL_NAMESPACE;

   // clang-format off
   struct a : one< 'a' > {};
   struct b : one< 'b' > {};
   struct grammar : sor< a, b > {};

   template< typename > inline constexpr const char* error_message = nullptr;
   template<> inline constexpr auto error_message< b > = "test123";
   // clang-format on

   struct error
   {
      template< typename Rule >
      static constexpr auto message = error_message< Rule >;
   };

   template< typename Rule >
   using control = must_if_n< error >::type< Rule >;

}  // namespace test1

namespace TAO_PEGTL_NAMESPACE
{
   namespace policy_constraints
   {
      struct no_message
      {
         template< typename Rule >
         static constexpr const char* message = nullptr;
      };

      struct with_message
      {
         template< typename Rule >
         static constexpr const char* message = "message";
      };

      struct override_false
         : with_message
      {
         template< typename Rule >
         static constexpr bool raise_on_failure = false;
      };

      struct override_true
      {
         template< typename Rule >
         static constexpr bool raise_on_failure = true;
      };

      static_assert( !internal::raise_on_failure< no_message, void > );
      static_assert( internal::raise_on_failure< with_message, void > );
      static_assert( !internal::raise_on_failure< override_false, void > );
      static_assert( internal::raise_on_failure< override_true, void > );
   }  // namespace policy_constraints

   void unit_test()
   {
      try {
         parse< test1::grammar, nothing, test1::control >( text_view_input< scan::lf >( "c" ) );
         TAO_PEGTL_TEST_UNREACHABLE;  // LCOV_EXCL_LINE
      }
      catch( const parse_error_base& e ) {
         TAO_PEGTL_TEST_ASSERT( e.message() == "test123" );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"

#endif
