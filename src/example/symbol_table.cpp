// Copyright (c) 2018-2026 Dr. Colin Hirsch and Daniel Frey
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

#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include <tao/pegtl.hpp>

#include <tao/pegtl/extra/charconv.hpp>

namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace example
{
   // Simple example for parsing with a symbol table; here the custom
   // logic is in the semantic actions and an exception is thrown on
   // error -- other possibilities are to let the actions' apply()
   // function return a bool instead, or to use a custom rule for the
   // matching of name within assignment that uses the symbol table
   // in its match() function.

   struct state
   {
      unsigned converted = 0;
      std::string temporary;
      std::map< std::string, unsigned > symbol_table;
   };

   // clang-format off
   struct semi : pegtl::one< ';' > {};
   struct blank0 : pegtl::star< pegtl::blank > {};
   struct blanks : pegtl::plus< pegtl::blank > {};
   struct name : pegtl::plus< pegtl::alpha > {};
   struct value : pegtl::plus< pegtl::digit > {};
   struct equals : pegtl::pad< pegtl::one< '=' >, pegtl::blank > {};
   struct definition : pegtl::if_must< pegtl::string< "def" >, blanks, name, blank0, semi > {};
   struct assignment : pegtl::if_must< name, equals, value, blank0, semi > {};
   struct something : pegtl::sor< pegtl::space, definition, assignment > {};
   struct grammar : pegtl::until< pegtl::eof, pegtl::must< something > > {};
   // clang-format on

   template< typename Rule >
   struct action
   {};

   template<>
   struct action< value >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, state& st )
      {
         (void)pegtl::from_chars_throws< void >::apply( in, st.converted );
      }
   };

   template<>
   struct action< name >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, state& st )
      {
         st.temporary = in.string();
      }
   };

   template<>
   struct action< definition >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, state& st )
      {
         if( !st.symbol_table.try_emplace( st.temporary, 0 ).second ) {
            pegtl::throw_parse_error( "duplicate symbol " + st.temporary, in );
         }
      }
   };

   template<>
   struct action< assignment >
   {
      template< typename ActionInput >
      static void apply( const ActionInput& in, state& st )
      {
         const auto i = st.symbol_table.find( st.temporary );
         if( i == st.symbol_table.end() ) {
            pegtl::throw_parse_error( "unknown symbol " + st.temporary, in );
         }
         i->second = st.converted;
      }
   };

}  // namespace example

int main( int argc, char** argv )
{
   for( int i = 1; i < argc; ++i ) {
      pegtl::text_file_input< pegtl::scan::lf_crlf > in( argv[ i ] );
      example::state st;
      pegtl::parse< example::grammar, example::action >( in, st );
      for( const auto& j : st.symbol_table ) {
         std::cout << j.first << " = " << j.second << std::endl;
      }
   }
   return 0;
}

#endif
