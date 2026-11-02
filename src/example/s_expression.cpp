// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#if !defined( __cpp_exceptions )
#include <iostream>
int main()
{
   std::cerr << "Exception support required, example unavailable." << std::endl;
   return 1;
}
#else

#include <iomanip>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/debug/analyze.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace example
{
   // clang-format off
   struct hash_comment : pegtl::until< pegtl::eolf > {};

   struct list;
   struct list_comment : pegtl::if_must< pegtl::at< pegtl::one< '(' > >, pegtl::disable< list > > {};

   struct read_include : pegtl::seq< pegtl::one< ' ' >, pegtl::one< '"' >, pegtl::star< pegtl::not_one7< '"' > >, pegtl::one< '"' > > {};
   struct hash_include : pegtl::if_must< pegtl::string< "include" >, read_include > {};

   struct hashed : pegtl::if_must< pegtl::one< '#' >, pegtl::sor< hash_include, list_comment, hash_comment > > {};

   struct number : pegtl::plus< pegtl::digit > {};
   struct symbol : pegtl::identifier {};

   struct atom : pegtl::sor< number, symbol > {};

   struct anything;

   struct list : pegtl::if_must< pegtl::one< '(' >, pegtl::until< pegtl::one< ')' >, anything > > {};

   struct normal : pegtl::sor< atom, list > {};

   struct anything : pegtl::sor< pegtl::space, hashed, normal > {};

   struct grammar : pegtl::until< pegtl::eof, pegtl::must< anything > > {};
   // clang-format on

   template< typename Rule >
   struct action
   {};

   template<>
   struct action< pegtl::star< pegtl::not_one7< '"' > > >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& fn )
      {
         fn = in.string();
      }
   };

   template<>
   struct action< hash_include >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, std::string& fn )
      {
         std::string f2;
         // Here f2 is the state argument for the nested parsing
         // run (to store the value of the string literal like in
         // the upper-level parsing run), fn is the value of the
         // last string literal that we use as filename here, and
         // the input is passed on for chained error messages (as
         // in "error in line x file foo included from file bar...)
         pegtl::text_file_input< pegtl::scan::lf_crlf > i2( fn );
         pegtl::parse_nested< grammar, example::action >( in, i2, f2 );
      }
   };

}  // namespace example

int main( int argc, char** argv )
{
   if( pegtl::analyze< example::grammar >() != 0 ) {
      return 1;
   }
   using input_t = pegtl::argv_input< pegtl::scan::lf_crlf >;
   for( int i = 1; i < argc; ++i ) {
      std::string fn;
      input_t in( argv, i );
      try {
         pegtl::parse< example::grammar, example::action >( in, fn );
      }
      catch( const pegtl::parse_error_base& e ) {
         std::cerr << e.what() << std::endl;
      }
   }
   return 0;
}

#endif
