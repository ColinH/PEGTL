// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_MEMBER_HPP
#define TAO_PEGTL_MEMBER_HPP

#include "config.hpp"

#include "internal/peek_member.hpp"
#include "internal/rules.hpp"

namespace TAO_PEGTL_NAMESPACE::member
{
   // clang-format off
   template< auto M, auto Function > struct function : internal::function< internal::peek_member< M >, Function > {};
   template< auto M, typename Rule > struct nested : internal::nested< internal::peek_member< M >, Rule > {};
   template< auto M, internal::peek_member< M >::data_t E, decltype( E )... Es > struct string : internal::seq_one< internal::peek_member< M >, E, Es... > {};

   template< auto M, internal::peek_member< M >::data_t E, decltype( E )... Es > struct not_one : internal::not_one< internal::peek_member< M >, E, Es... > {};
   template< auto M, internal::peek_member< M >::data_t Lo, decltype( Lo ) Hi > struct not_range : internal::not_range< internal::peek_member< M >, Lo, Hi > {};
   template< auto M, internal::peek_member< M >::data_t E, decltype( E )... Es > struct not_ranges : internal::not_ranges< internal::peek_member< M >, E, Es... > {};
   template< auto M, internal::peek_member< M >::data_t E, decltype( E )... Es > struct one : internal::one< internal::peek_member< M >, E, Es... > {};
   template< auto M, internal::peek_member< M >::data_t Lo, decltype( Lo ) Hi > struct range : internal::range< internal::peek_member< M >, Lo, Hi > {};
   template< auto M, internal::peek_member< M >::data_t E, decltype( E )... Es > struct ranges : internal::ranges< internal::peek_member< M >, E, Es... > {};
   // clang-format on
}  // namespace TAO_PEGTL_NAMESPACE::member

// FCOV_EXCL_FILE
#endif
