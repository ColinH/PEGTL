// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_EXAMPLE_JSON_HPP
#define TAO_PEGTL_EXAMPLE_JSON_HPP

#include "../ascii.hpp"
#include "../config.hpp"
#include "../rules.hpp"
#include "../unicode/utf8.hpp"

#include "escaped.hpp"

namespace TAO_PEGTL_NAMESPACE::json
{
   // JSON grammar according to RFC 8259

   // clang-format off
   struct ws : one< ' ', '\t', '\n', '\r' > {};

   template< typename R, typename P = ws >
   struct padr : seq< R, star< P > > {};

   struct begin_array : padr< one< '[' > > {};
   struct begin_object : padr< one< '{' > > {};
   struct end_array : one< ']' > {};
   struct end_object : one< '}' > {};
   struct name_separator : pad< one< ':' >, ws > {};
   struct value_separator : padr< one< ',' > > {};

   struct false_ : string< "false" > { using case_t = one< 'f' >; };  // NOLINT(readability-identifier-naming)
   struct null : string< "null" > { using case_t = one< 'n' >; };
   struct true_ : string< "true" > { using case_t = one< 't' >; };  // NOLINT(readability-identifier-naming)

   struct digits : plus< digit > {};
   struct exp : seq< one< 'e', 'E' >, opt< one< '-', '+' > >, digits > {};
   struct frac : seq< one< '.' >, digits > {};
   struct int_ : sor< one< '0' >, plus< digit > > {};  // NOLINT(readability-identifier-naming)
   struct number : seq< opt< one< '-' > >, int_, opt< frac >, opt< exp > > { using case_t = any; };

   struct char_ : json_character {};  // NOLINT(readability-identifier-naming)

   struct string_content : until< at< one< '"' > >, char_ > {};
   struct string : seq< one< '"' >, string_content, consume< 1 > >
   {
      using case_t = one< '"' >;
      using content = string_content;
   };

   struct key_content : until< at< one< '"' > >, char_ > {};
   struct key : seq< one< '"' >, key_content, consume< 1 > >
   {
      using content = key_content;
   };

   struct value;

   struct array_element;
   struct next_array_element : seq< array_element > {};
   struct array_content : opt< array_element, star< value_separator, next_array_element > > {};
   struct array : seq< begin_array, array_content, end_array >
   {
      using case_t = one< '[' >;
      using begin = begin_array;
      using end = end_array;
      using element = array_element;
      using content = array_content;
   };

   struct member_value : padr< value > {};
   struct member : seq< key, name_separator, member_value > {};
   struct next_member : seq< member > {};
   struct object_content : opt< member, star< value_separator, next_member > > {};
   struct object : seq< begin_object, object_content, end_object >
   {
      using case_t = one< '{' >;
      using begin = begin_object;
      using end = end_object;
      using element = member;
      using content = object_content;
   };

   struct value : sor1< null, true_, false_, array, object, string, number > {};
   struct array_element : padr< value > {};

   struct text : pad< value, ws > {};
   // clang-format on

}  // namespace TAO_PEGTL_NAMESPACE::json

// FCOV_EXCL_FILE
#endif
