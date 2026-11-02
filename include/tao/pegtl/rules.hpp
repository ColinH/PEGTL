// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_RULES_HPP
#define TAO_PEGTL_RULES_HPP

#include <cstddef>

#if defined( __cpp_exceptions )
#include <exception>

#include "parse_error_base.hpp"
#endif

#include "config.hpp"

#include "internal/fixed_string.hpp"
#include "internal/rules.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   // clang-format off
   template< template< typename... > class Action, typename... Rules > struct action : internal::action< Action, Rules... > {};
   template< typename... Actions > struct apply : internal::apply< Actions... > {};
   template< typename... Actions > struct apply0 : internal::apply0< Actions... > {};
   template< typename... Rules > struct at : internal::at< Rules... > {};
   struct bof : internal::bof {};
   struct bol : internal::bol {};
   template< std::size_t Count > struct consume : internal::consume< Count > {};
   template< template< typename... > class Control, typename... Rules > struct control : internal::control< Control, Rules... > {};
   template< typename... Rules > struct disable : internal::disable< Rules... > {};
   template< typename... Rules > struct enable : internal::enable< Rules... > {};
   struct eof : internal::eof {};
   struct eol : internal::eol {};
   struct eolf : internal::eolf {};
   struct everything : internal::everything {};
   struct failure : internal::failure {};
   template< auto Function, typename Peek = void > struct function : internal::function< Peek, Function > {};
   template< typename Rule, typename... Actions > struct if_apply : internal::if_apply< Rule, Actions... > {};
   template< typename Cond, typename Then, typename Else > struct if_then_else : internal::if_then_else< Cond, Then, Else > {};
   template< typename Rule > struct invert : internal::invert_traits< typename Rule::rule_t >::type {};
   template< typename Rule, typename Sep, typename Pad = void > struct list : internal::list< Rule, internal::pad< Sep, Pad > > {};
   template< typename Rule, typename Sep > struct list< Rule, Sep, void > : internal::list< Rule, Sep > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list_opt : internal::list_opt< Rule, internal::pad< Sep, Pad > > {};
   template< typename Rule, typename Sep > struct list_opt< Rule, Sep, void > : internal::list_opt< Rule, Sep > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list_tail : internal::list_tail_pad< Rule, Sep, Pad > {};
   template< typename Rule, typename Sep > struct list_tail< Rule, Sep, void > : internal::list_tail< Rule, Sep > {};
   template< typename M, typename S > struct minus : internal::minus< M, S > {};
   template< typename Rule, typename Peek > struct nested : internal::nested< Peek, Rule > {};
   template< typename... Rules > struct not_at : internal::not_at< Rules... > {};
   template< typename... Rules > struct opt : internal::opt< Rules... > {};
   template< typename Rule, typename Pad1, typename Pad2 = Pad1 > struct pad : internal::pad< Rule, Pad1, Pad2 > {};
   template< typename Rule, typename Pad > struct pad_opt : internal::pad_opt< Rule, Pad > {};
   template< typename Rule, typename... Rules > struct partial : internal::partial< Rule, Rules... > {};
   template< typename Rule, typename... Rules > struct plus : internal::plus< Rule, Rules... > {};
   template< typename Head, typename... Rules > struct rematch : internal::rematch< Head, Rules... > {};
   template< std::size_t Num, typename... Rules > struct rep : internal::rep< Num, Rules... > {};
   template< std::size_t Max, typename... Rules > struct rep_max : internal::rep_min_max< 0, Max, Rules... > {};
   template< std::size_t Min, typename Rule, typename... Rules > struct rep_min : internal::rep_min< Min, Rule, Rules... > {};
   template< std::size_t Min, std::size_t Max, typename... Rules > struct rep_min_max : internal::rep_min_max< Min, Max, Rules... > {};
   template< std::size_t Max, typename... Rules > struct rep_opt : internal::rep_opt< Max, Rules... > {};
   struct restart : internal::restart {};
   template< typename Sep, typename... Rules > struct separated : internal::separated< type_list<>, Sep, Rules... > {};
   template< typename Sep, typename Pad, typename... Rules > struct separated_pad : internal::separated_pad< type_list<>, Sep, Pad, Rules... > {};
   template< typename... Rules > struct seq : internal::seq< Rules... > {};
   template< typename... Rules > struct sor : internal::sor< Rules... > {};
   template< typename... Rules > struct sor1 : internal::sor1< Rules... > {};
   template< typename Rule > struct source : internal::source< Rule > {};
   template< typename Rule, typename... Rules > struct star : internal::star< Rule, Rules... > {};
   template< typename Rule, typename... Rules > struct star_partial : internal::star_partial< Rule, Rules... > {};
   template< typename Rule, typename... Rules > struct star_strict : internal::star_strict< Rule, Rules... > {};
   template< typename State, typename... Rules > struct state : internal::state< State, Rules... > {};
   template< typename Rule, typename... Rules > struct strict : internal::strict< Rule, Rules... > {};
   struct success : internal::success {};
   template< typename... Rules > struct unordered : internal::unordered< false, Rules... > {};
   template< typename... Rules > struct unordered_partial : internal::unordered< true, Rules... > {};
   template< typename Cond, typename... Rules > struct until : internal::until< Cond, Rules... > {};

#if defined( __cpp_exceptions )
   template< typename Cond, typename... Thens > struct if_must : internal::if_must< false, Cond, Thens... > {};
   template< typename Cond, typename Then, typename Else > struct if_must_else : internal::if_must_else< Cond, Then, Else > {};
   template< typename Rule, typename Sep, typename Pad = void > struct list_must : internal::list_must< Rule, internal::pad< Sep, Pad > > {};
   template< typename Rule, typename Sep > struct list_must< Rule, Sep, void > : internal::list_must< Rule, Sep > {};
   template< typename... Rules > struct must : internal::must< Rules... > {};
   template< typename Cond, typename... Rules > struct opt_must : internal::if_must< true, Cond, Rules... > {};
   template< typename Exception > struct raise : internal::raise< Exception > {};
   template< internal::fixed_string Text > struct raise_message : internal::raise< raise_message< Text > > {
      static constexpr auto& error_message = Text.data;
   };
   template< char... Cs > struct raise_message_chars : internal::raise< raise_message_chars< Cs... > > {
      static constexpr const char error_message[] = { Cs..., 0 };
   };
   template< typename Cond, typename... Rules > struct star_must : internal::star_must< Cond, Rules... > {};
   template< typename... Rules > struct try_catch_raise_nested : internal::try_catch_raise_nested< parse_error_base, Rules... > {};
   template< typename... Rules > struct try_catch_return_false : internal::try_catch_return_false< parse_error_base, Rules... > {};
   template< typename... Rules > struct try_catch_any_raise_nested : internal::try_catch_raise_nested< void, Rules... > {};
   template< typename... Rules > struct try_catch_any_return_false : internal::try_catch_return_false< void, Rules... > {};
   template< typename... Rules > struct try_catch_std_raise_nested : internal::try_catch_raise_nested< std::exception, Rules... > {};
   template< typename... Rules > struct try_catch_std_return_false : internal::try_catch_return_false< std::exception, Rules... > {};
   template< typename Exception, typename... Rules > struct try_catch_type_raise_nested : internal::try_catch_raise_nested< Exception, Rules... > {};
   template< typename Exception, typename... Rules > struct try_catch_type_return_false : internal::try_catch_return_false< Exception, Rules... > {};
#endif
   // clang-format on

}  // namespace TAO_PEGTL_NAMESPACE

// FCOV_EXCL_FILE
#endif
