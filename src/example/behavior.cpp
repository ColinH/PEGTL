// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
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

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <tao/pegtl.hpp>

// Prints the behavior tables in markdown format for the PEGTL documentation.

namespace TAO_PEGTL_NAMESPACE::example
{
#if defined( _MSC_VER )
#pragma warning( push )
#pragma warning( disable : 4310 )
#endif

   void print( const std::string& s )
   {
      if( ( s.size() == 1 ) && static_cast< bool >( s.at( 0 ) & static_cast< char >( 0x80 ) ) ) {
         std::cout << " \"\\xA4\" |";
      }
      else {
         std::cout << " \"" << s << "\" |";
      }
   }

#if defined( _MSC_VER )
#pragma warning( pop )
#endif

   void header( const std::vector< std::string >& v )
   {
      std::cout << "|  |";
      for( const auto& s : v ) {
         print( s );
      }
      std::cout << "\n|--|";
      for( const auto& s : v ) {
         (void)s;
         std::cout << "--|";
      }
      std::cout << "\n";
   }

   template< typename Rule >
   void row( const std::string& r, const std::vector< std::string >& v )
   {
      std::cout << "| `" << r << "` |";
      for( const auto& s : v ) {
         view_input<> in( s );
         const char* a = in.current();
         try {
            if( parse< Rule >( in ) ) {
               const std::string t( a, in.current() - a );
               print( t );
            }
            else {
               std::cout << " f |";
            }
         }
         catch( const parse_error_base& /*unused*/ ) {
            std::cout << " E |";
         }
      }
      std::cout << "\n";
   }

   void table1()
   {
      using a = one< 'a' >;
      using b = one< 'b' >;

      std::cout << "## Simple Combinators\n\n";
      {
         const std::vector< std::string > v = { "", "a", "ab", "z", "az", "abc" };
         header( v );
         row< seq< a, b > >( "seq< a, b >", v );
         row< opt< a, b > >( "opt< a, b >", v );
         row< opt_must< a, b > >( "opt_must< a, b >", v );
         row< strict< a, b > >( "strict< a, b >", v );
         row< partial< a, b > >( "partial< a, b >", v );
         row< must< a, b > >( "must< a, b >", v );
         row< if_must< a, b > >( "if_must< a, b >", v );
      }
      std::cout << std::endl;
   }

   void table2()
   {
      using a = one< 'a' >;
      using b = one< 'b' >;

      std::cout << "## Iterating Combinators\n\n";
      {
         const std::vector< std::string > v = { "", "a", "ab", "aba", "abab", "z", "az", "abz", "abaz" };
         header( v );
         row< plus< a, b > >( "plus< a, b >", v );
         row< star< a, b > >( "star< a, b >", v );
         row< star_strict< a, b > >( "star_strict< a, b >", v );
         row< star_partial< a, b > >( "star_partial< a, b >", v );
         row< star_must< a, b > >( "star_must< a, b >", v );
      }
      std::cout << std::endl;
   }

   void table3()
   {
      using a = one< 'a' >;

      std::cout << "## Repeating Combinators\n\n";
      {
         const std::vector< std::string > v = { "", "a", "aa", "aaa", "aaaa" };
         header( v );
         row< rep< 2, a > >( "rep< 2, a >", v );
         row< rep_opt< 2, a > >( "rep_opt< 2, a >", v );
         row< rep_min< 2, a > >( "rep_min< 2, a >", v );
         row< rep_max< 2, a > >( "rep_max< 2, a >", v );
         row< rep_min_max< 2, 3, a > >( "rep_min_max< 2, 3, a >", v );
      }
      std::cout << std::endl;
   }

   void table4()
   {
      using a = one< 'a' >;
      using b = one< 'b' >;
      using c = one< 'c' >;

      std::cout << "## List Combinators\n\n";
      {
         const std::vector< std::string > v = { "", "a", "aa", "ab", "aba", "abab", "abc", "ac", "acb", "acba", "acbca" };
         header( v );
         row< list< a, b > >( "list< a, b >", v );
         row< list_opt< a, b > >( "list_opt< a, b >", v );
         row< list_tail< a, b > >( "list_tail< a, b >", v );
         row< list_must< a, b > >( "list_must< a, b >", v );
         row< list< a, b, c > >( "list< a, b, c >", v );
         row< list_opt< a, b, c > >( "list_opt< a, b, c >", v );
         row< list_tail< a, b, c > >( "list_tail< a, b, c >", v );
         row< list_must< a, b, c > >( "list_must< a, b, c >", v );
      }
      std::cout << std::endl;
   }

   void table5()
   {
      static constexpr char c = 'c';
      static constexpr char g = 'g';

      std::cout << "## ASCII Matching Rules\n\n";
      {
         const std::vector< std::string > v = { "a", "c", "e", "G", "Z", "\xa4" };
         header( v );
         row< any >( "any", v );
         row< one< c, g > >( "one< 'c', 'g' >", v );
         row< ione< c, g > >( "ione< 'c', 'g' >", v );
         row< range< c, g > >( "range< 'c', 'g' >", v );
         row< not_one< c, g > >( "not_one< 'c', 'g' >", v );
         row< not_ione< c, g > >( "not_ione< 'c', 'g' >", v );
         row< not_range< c, g > >( "not_range< 'c', 'g' >", v );
         row< any7 >( "any7", v );
         row< not_one7< c, g > >( "not_one7< 'c', 'g' >", v );
         row< not_ione7< c, g > >( "not_ione7< 'c', 'g' >", v );
         row< not_range7< c, g > >( "not_range7< 'c', 'g' >", v );
      }
      std::cout << std::endl;
   }

   void table6()
   {
      std::cout << "## ASCII String Rules\n\n";
      {
         const std::vector< std::string > v = { "", "inf", "infinityx", "iNFi", "infiNiTy" };
         header( v );
         row< string< "infinity" > >( "string< \"infinity\" >", v );
         row< istring< "infinity" > >( "istring< \"infinity\" >", v );
         row< astring< 3, "infinity" > >( "astring< 3, \"infinity\" >", v );
         row< aistring< 3, "infinity" > >( "aistring< 3, \"infinity\" >", v );
      }
   }

   void behavior()
   {
      table1();
      table2();
      table3();
      table4();
      table5();
      table6();
   }

}  // namespace TAO_PEGTL_NAMESPACE::example

int main()
{
   TAO_PEGTL_NAMESPACE::example::behavior();
   return 0;
}

#endif
