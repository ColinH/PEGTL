// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <string>

#include <tao/pegtl/internal/is_simple_type.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   enum class test_enum
   {
   };

   static_assert( internal::is_simple_type_v< int > );
   static_assert( internal::is_simple_type_v< long > );
   static_assert( internal::is_simple_type_v< char32_t > );
   static_assert( internal::is_simple_type_v< std::byte > );
   static_assert( internal::is_simple_type_v< test_enum > );

   static_assert( !internal::is_simple_type_v< int& > );
   static_assert( !internal::is_simple_type_v< long& > );
   static_assert( !internal::is_simple_type_v< char32_t& > );
   static_assert( !internal::is_simple_type_v< std::byte& > );
   static_assert( !internal::is_simple_type_v< test_enum& > );

   static_assert( !internal::is_simple_type_v< int* > );
   static_assert( !internal::is_simple_type_v< long* > );
   static_assert( !internal::is_simple_type_v< char32_t* > );
   static_assert( !internal::is_simple_type_v< std::byte* > );
   static_assert( !internal::is_simple_type_v< test_enum* > );

   static_assert( !internal::is_simple_type_v< void > );
   static_assert( !internal::is_simple_type_v< float > );
   static_assert( !internal::is_simple_type_v< std::string > );

   static_assert( internal::is_simple_type_v< bool > );  // TODO?

}  // namespace TAO_PEGTL_NAMESPACE

int main()
{
   return 0;
}
