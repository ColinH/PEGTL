// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_ASCII_HPP
#define TAO_PEGTL_ASCII_HPP

#include <cstdlib>

#include "config.hpp"

#include "internal/fixed_string.hpp"
#include "internal/peek_direct.hpp"
#include "internal/peek_seven.hpp"
#include "internal/rules.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   // TODO: C++20 namespace TAO_PEGTL_NAMESPACE::inline ascii

   inline namespace ascii
   {
      // clang-format off
      struct alnum : internal::ranges< internal::peek_char, 'a', 'z', 'A', 'Z', '0', '9' > {};
      struct alpha : internal::ranges< internal::peek_char, 'a', 'z', 'A', 'Z' > {};
      struct bdigit : internal::one< internal::peek_char, '0', '1' > {};
      struct blank : internal::one< internal::peek_char, ' ', '\t' > {};
      struct cntrl : internal::ranges< internal::peek_char, static_cast< char >( 0 ), static_cast< char >( 31 ), static_cast< char >( 127 ) > {};
      struct digit : internal::range< internal::peek_char, '0', '9' > {};
      struct esc : internal::one< internal::peek_char, static_cast< char >( 27 ) > {};
      struct ellipsis : internal::ascii_string< '.', '.', '.' > {};
      struct ff : internal::one< internal::peek_char, '\f' > {};
      struct graph : internal::range< internal::peek_char, static_cast< char >( 33 ), static_cast< char >( 126 ) > {};
      struct ht : internal::one< internal::peek_char, '\t' > {};
      struct lower : internal::range< internal::peek_char, 'a', 'z' > {};
      struct nul : internal::one< internal::peek_char, static_cast< char >( 0 ) > {};
      struct odigit : internal::range< internal::peek_char, '0', '7' > {};
      struct print : internal::range< internal::peek_char, static_cast< char >( 32 ), static_cast< char >( 126 ) > {};
      struct punct : internal::ranges< internal::peek_char, '!', '/', ':', '@', '[', '`', '{', '~' > {};
      struct sp : internal::one< internal::peek_char, ' ' > {};
      struct space : internal::one< internal::peek_char, ' ', '\n', '\r', '\t', '\v', '\f' > {};
      struct upper : internal::range< internal::peek_char, 'A', 'Z' > {};
      struct vt : internal::one< internal::peek_char, '\v' > {};
      struct xdigit : internal::ranges< internal::peek_char, '0', '9', 'a', 'f', 'A', 'F' > {};

      struct cr : internal::one< internal::peek_char, '\r' > {};
      struct lf : internal::one< internal::peek_char, '\n' > {};
      struct crlf : internal::ascii_string< '\r', '\n' > {};
      struct cr_lf : internal::one< internal::peek_char, '\r', '\n' > {};
      struct cr_crlf : internal::sor< crlf::rule_t, cr::rule_t > {};
      struct lf_crlf : internal::sor< lf::rule_t, crlf::rule_t > {};
      struct cr_lf_crlf : internal::sor< crlf::rule_t, cr_lf::rule_t > {};

      struct identifier_first : internal::identifier_first< internal::peek_char > {};
      struct identifier_other : internal::identifier_other< internal::peek_char > {};
      struct identifier : internal::identifier< internal::peek_char > {};
      template< internal::fixed_string Text > struct keyword : internal::string_to_chars< Text >::template apply< internal::ascii_keyword > {};
      template< char... Cs > struct keyword_chars : internal::ascii_keyword< Cs... > {};
      struct shebang : internal::seq< internal::ascii_string< '#', '!' >, internal::until< internal::eolf > > {};

      struct any : internal::any< internal::peek_char > {};
      template< std::size_t Prefix, internal::fixed_string Text > struct astring : internal::string_to_chars< Text >::template apply_prefix< internal::ascii_astring, Prefix > {};
      template< std::size_t Prefix, char... Cs > struct astring_chars : internal::ascii_astring< Prefix, Cs... > {};
      template< std::size_t Prefix, internal::fixed_string Text > struct aistring : internal::string_to_chars< Text >::template apply_prefix< internal::ascii_aistring, Prefix > {};
      template< std::size_t Prefix, char... Cs > struct aistring_chars : internal::ascii_aistring< Prefix, Cs... > {};
      template< internal::fixed_string Text > struct istring : internal::string_to_chars< Text >::template apply< internal::ascii_istring > {};
      template< char... Cs > struct istring_chars : internal::ascii_istring< Cs... > {};
      template< std::size_t Count > struct many : internal::many< Count, internal::peek_char > {};
      template< internal::fixed_string Text > struct string : internal::string_to_chars< Text >::template apply< internal::ascii_string > {};
      template< char... Cs > struct string_chars : internal::ascii_string< Cs... > {};
      template< char C > struct three : internal::ascii_string< C, C, C > {};
      template< char C > struct two : internal::ascii_string< C, C > {};

      template< char... Cs > struct ione : internal::ione< internal::peek_char, Cs... > {};
      template< char... Cs > struct not_ione : internal::not_ione< internal::peek_char, Cs... > {};
      template< char... Cs > struct not_one : internal::not_one< internal::peek_char, Cs... > {};
      template< char Lo, char Hi > struct not_range : internal::not_range< internal::peek_char, Lo, Hi > {};
      template< char... Cs > struct not_ranges : internal::not_ranges< internal::peek_char, Cs... > {};
      template< char... Cs > struct one : internal::one< internal::peek_char, Cs... > {};
      template< char Lo, char Hi > struct range : internal::range< internal::peek_char, Lo, Hi > {};
      template< char... Cs > struct ranges : internal::ranges< internal::peek_char, Cs... > {};

      struct any7 : internal::any< internal::peek_seven > {};
      template< std::size_t Count > struct many7 : internal::many< Count, internal::peek_seven > {};
      template< char... Cs > struct not_ione7 : internal::not_ione< internal::peek_seven, Cs... > {};
      template< char... Cs > struct not_one7 : internal::not_one< internal::peek_seven, Cs... > {};
      template< char Lo, char Hi > struct not_range7 : internal::not_range< internal::peek_seven, Lo, Hi > {};
      // clang-format on

      namespace scan
      {
         // clang-format off
         struct cr : ascii::cr { using eol_char_rule = ascii::cr; };
         struct lf : ascii::lf { using eol_char_rule = ascii::lf; };
         struct lf_crlf : ascii::lf_crlf { using eol_char_rule = ascii::lf; };
         // clang-format on
      }  // namespace scan

      namespace lazy
      {
         // clang-format off
         struct cr : ascii::cr { using eol_lazy_peek = internal::peek_char; };
         struct lf : ascii::lf { using eol_lazy_peek = internal::peek_char; };
         struct crlf : ascii::crlf { using eol_lazy_peek = internal::peek_char; };
         struct cr_lf : ascii::cr_lf { using eol_lazy_peek = internal::peek_char; };
         struct cr_crlf : ascii::cr_crlf { using eol_lazy_peek = internal::peek_char; };
         struct lf_crlf : ascii::lf_crlf { using eol_lazy_peek = internal::peek_char; };
         struct cr_lf_crlf : ascii::cr_lf_crlf { using eol_lazy_peek = internal::peek_char; };
         // clang-format on
      }  // namespace lazy

   }  // namespace ascii

}  // namespace TAO_PEGTL_NAMESPACE

// FCOV_EXCL_FILE
#endif
