// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXAMPLE_ABNF_ABNF_HPP
#define TAO_PEGTL_EXAMPLE_ABNF_ABNF_HPP

#include "../ascii.hpp"
#include "../config.hpp"
#include "../rules.hpp"

namespace TAO_PEGTL_NAMESPACE::abnf
{
   // ABNF grammar according to RFC 5234, updated by RFC 7405, with
   // the following differences:
   //
   // To form a C++ identifier from a rulename, all minuses must be
   // replaced with underscores.
   //
   // As C++ identifiers are case-sensitive, we remember the "correct"
   // spelling from the first occurrence of a rulename, all other
   // occurrences are automatically changed to that.
   //
   // Certain rulenames are reserved as their equivalent C++ identifier is
   // reserved as a keyword, an alternative token, by the standard or
   // for other, special reasons.
   //
   // When using numerical values (num-val, repeat), the values
   // must be in the range of the corresponding C++ data type.
   //
   // Remember we are defining a PEG, not a CFG. Simply copying some
   // ABNF from somewhere might lead to surprising results as the
   // alternatives are ordered and determinstic, using the sor<> rule.
   //
   // PEGs also require two extensions: The and-predicate and the
   // not-predicate. They are expressed by '&' and '!' respectively,
   // being allowed (optionally, only one of them) before the
   // repetition. You can use braces for more complex expressions.
   //
   // Finally, instead of the pre-defined CRLF sequence, we use the
   // eol rule which accepts whatever the input's Eol is set to.

   // clang-format off
   struct dquote : internal::one< internal::peek_char, '"' > {};

   struct comment_cont : until< eol, sor< ascii::blank, ascii::graph > > {};
   struct comment : seq< one< ';' >, comment_cont > {};
   struct c_nl : sor< comment, eol > {};
   struct req_c_nl : c_nl {};
   struct c_wsp : sor< ascii::blank, seq< c_nl, ascii::blank > > {};

   struct rulename : seq< ascii::alpha, star< ascii::ranges< 'a', 'z', 'A', 'Z', '0', '9', '-' > > > {};

   struct quoted_string_cont : until< dquote, ascii::print > {};
   struct quoted_string : seq< dquote, quoted_string_cont > {};
   struct case_insensitive_string : seq< opt< ascii::istring< "%i" > >, quoted_string > {};
   struct case_sensitive_string : seq< ascii::istring< "%s" >, quoted_string > {};
   struct char_val : sor< case_insensitive_string, case_sensitive_string > {};

   struct prose_val_cont : until< ascii::one< '>' >, ascii::print > {};
   struct prose_val : seq< ascii::one< '<' >, prose_val_cont > {};

   template< char First, typename Digit >
   struct gen_val
   {
      struct value : plus< Digit > {};
      struct range : seq< ascii::one< '-' >, value > {};
      struct next_value : seq< value > {};
      struct type : seq< ascii::istring_chars< First >, value, sor< range, star< ascii::one< '.' >, next_value > > > {};
   };

   using hex_val = gen_val< 'x', ascii::xdigit >;
   using dec_val = gen_val< 'd', ascii::digit >;
   using bin_val = gen_val< 'b', ascii::bdigit >;

   struct num_val_choice : sor< bin_val::type, dec_val::type, hex_val::type > {};
   struct num_val : seq< ascii::one< '%' >, num_val_choice > {};

   struct alternative;
   struct option_close : ascii::one< ']' > {};
   struct option : seq< ascii::one< '[' >, pad< alternative, c_wsp >, option_close > {};
   struct group_close : ascii::one< ')' > {};
   struct group : seq< ascii::one< '(' >, pad< alternative, c_wsp >, group_close > {};
   struct element : sor< rulename, group, option, char_val, num_val, prose_val > {};

   struct repeat : sor< seq< star< ascii::digit >, ascii::one< '*' >, star< ascii::digit > >, plus< ascii::digit > > {};
   struct repetition : seq< opt< repeat >, element > {};
   struct req_repetition : seq< repetition > {};

   struct and_predicate : seq< ascii::one< '&' >, req_repetition > {};
   struct not_predicate : seq< ascii::one< '!' >, req_repetition > {};
   struct predicate : sor< and_predicate, not_predicate, repetition > {};

   struct concatenation : list< predicate, plus< c_wsp > > {};
   struct alternative : list< concatenation, pad< ascii::one< '/' >, c_wsp > > {};

   struct defined_as_op : sor< ascii::string< "=/" >, ascii::one< '=' > > {};
   struct defined_as : pad< defined_as_op, c_wsp > {};
   struct rule : seq< seq< rulename, defined_as, alternative >, star< c_wsp >, req_c_nl > {};
   struct rulelist : until< eof, sor< seq< star< c_wsp >, c_nl >, rule > > {};
   // clang-format on

}  // namespace TAO_PEGTL_NAMESPACE::abnf

// FCOV_EXCL_FILE
#endif
