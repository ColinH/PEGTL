// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <tao/pegtl.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace example
{
   // clang-format off
   struct prefix : pegtl::string< "Hello, " > {};
   struct name : pegtl::plus< pegtl::alpha > {};
   struct grammar : pegtl::seq< prefix, name, pegtl::one< '!' >, pegtl::eof > {};
   // clang-format on

   template< typename Rule >
   struct action
      : pegtl::nothing< Rule >
   {};

   template<>
   struct action< name >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& v )
      {
         v = in.string();
      }
   };

}  // namespace example

int main( int argc, char** argv )
{
   if( argc < 2 ) {
      return 1;
   }
   std::string name;
   pegtl::argv_input in( argv, 1 );

   if( !pegtl::parse< example::grammar, example::action >( in, name ) ) {
      std::cerr << "I can't parse you!" << std::endl;
      return 1;
   }
   std::cout << "Good bye, " << name << "!" << std::endl;
   return 0;
}
