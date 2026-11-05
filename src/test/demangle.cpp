// Copyright (c) 2017-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <iostream>
#include <string>
#include <typeinfo>

#define TAO_PEGTL_STRINGIFY( a ) TAO_PEGTL_STRINGIFY_IMPL( a )
#define TAO_PEGTL_STRINGIFY_IMPL( a ) #a

namespace TAO_PEGTL_NAMESPACE
{
   template< typename T >
   void test1( const std::string& s )
   {
      const auto d = demangle< T >();
      if( d != s ) {
         std::cerr << "REFERENCE: " << s << std::endl;
         std::cerr << "DEMANGLED: " << d << std::endl;
      }
      TAO_PEGTL_TEST_ASSERT( d == s );
   }

#if defined( __cpp_rtti )
   template< typename T >
   void test_typeid()
   {
      test1< T >( typeid( T ).name() );
   }
#endif

   void unit_test()
   {
      const std::string ns = TAO_PEGTL_STRINGIFY( TAO_PEGTL_NAMESPACE );
#if !defined( __clang__ ) && defined( __GNUC__ ) && ( __GNUC__ == 16 ) && ( __GNUC_MINOR__ < 2 )
      // See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91155
#if defined( __cpp_rtti )
      test_typeid< int >();
      test_typeid< double >();
      test_typeid< seq< consume< 42 >, eof > >();
#endif
#elif defined( _MSC_VER ) && !defined( __clang__ )
      test1< int >( "int" );
      test1< double >( "double" );
      // In the Microsoft world, class and struct are not the same!
      test1< seq< many7< 42 >, eof > >( "struct " + ns + "::seq<struct " + ns + "::ascii::many7<42>,struct " + ns + "::eof>" );
#else
      test1< int >( "int" );
      test1< double >( "double" );
      test1< seq< consume< 42 >, eof > >( ns + "::seq<" + ns + "::consume<42>, " + ns + "::eof>" );
      // Some compilers include inline namespace ascii: test1< seq< many7< 42 >, eof > >( ns + "::seq<" + ns + "::many7<42>, " + ns + "::eof>" );
#endif
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
