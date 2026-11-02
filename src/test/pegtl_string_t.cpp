// Copyright (c) 2015-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>

#include <tao/pegtl.hpp>

namespace test
{
   namespace pegtl = TAO_PEGTL_NAMESPACE;

   // We only need to test that this compiles...

   struct foo
      : pegtl::string< "foo" >
   {};

   struct foobar
      : pegtl::sor< pegtl::string< "foo" >, pegtl::string< "bar" > >
   {};

   template< typename Literal, typename Chars >
   constexpr bool same_matcher = !std::is_same_v< Literal, Chars > && std::is_same_v< typename Literal::rule_t, typename Chars::rule_t >;

   static_assert( same_matcher< pegtl::string< "Hello" >, pegtl::string_chars< 'H', 'e', 'l', 'l', 'o' > > );
   static_assert( !std::is_same_v< pegtl::istring< "Hello" >::rule_t, pegtl::string< "Hello" >::rule_t > );
   static_assert( same_matcher< pegtl::istring< "Hello" >, pegtl::istring_chars< 'H', 'e', 'l', 'l', 'o' > > );

   static_assert( same_matcher< pegtl::keyword< "private" >, pegtl::keyword_chars< 'p', 'r', 'i', 'v', 'a', 't', 'e' > > );

   // Strings may even contain embedded null bytes

   static_assert( same_matcher< pegtl::string< "Hello, w\0rld!" >, pegtl::string_chars< 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 0, 'r', 'l', 'd', '!' > > );

   // Longer literals use the same matchers as character packs.

   static_assert( same_matcher< pegtl::string< "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" >,
                                pegtl::string_chars< 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' > > );

}  // namespace test

int main()
{
   return 0;
}
