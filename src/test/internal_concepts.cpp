// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string_view>

#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/nothing.hpp>

#include <tao/pegtl/internal/has_apply.hpp>
#include <tao/pegtl/internal/has_apply0.hpp>
#include <tao/pegtl/internal/has_eol_lazy_peek.hpp>
#include <tao/pegtl/internal/has_error_message.hpp>
#include <tao/pegtl/internal/has_success.hpp>
#include <tao/pegtl/internal/has_unwind.hpp>

namespace TAO_PEGTL_NAMESPACE::concept_tests
{
   struct missing
   {};

   template< typename Result >
   struct hooks
   {
      template< template< typename... > class Action >
      static Result apply( int& );

      template< template< typename... > class Action >
      static Result apply0( int& );

      Result success( int& ) &&;

      static Result unwind( const int&, int&& );
   };

   // Exact result types and argument/receiver reference qualifiers are significant.
   static_assert( internal::has_apply< hooks< void >, void, nothing, int& > );
   static_assert( internal::has_apply< hooks< bool >, bool, nothing, int& > );
   static_assert( !internal::has_apply< hooks< void >, bool, nothing, int& > );
   static_assert( !internal::has_apply< hooks< int >, bool, nothing, int& > );
   static_assert( internal::has_apply< hooks< int& >, int&, nothing, int& > );
   static_assert( !internal::has_apply< hooks< int& >, int, nothing, int& > );

   static_assert( internal::has_apply0< hooks< void >, void, nothing, int& > );
   static_assert( internal::has_apply0< hooks< bool >, bool, nothing, int& > );
   static_assert( !internal::has_apply0< hooks< bool >, void, nothing, int& > );
   static_assert( !internal::has_apply0< hooks< int >, bool, nothing, int& > );

   static_assert( internal::has_success< hooks< void >, void, int& > );
   static_assert( !internal::has_success< hooks< bool >, void, int& > );

   static_assert( internal::has_unwind< hooks< void >, void, const int&, int > );
   static_assert( !internal::has_unwind< hooks< bool >, void, const int&, int > );

   static_assert( !internal::has_apply< hooks< void >, void, nothing, int > );
   static_assert( !internal::has_apply0< hooks< void >, void, nothing, const int& > );
   static_assert( !internal::has_success< hooks< void >&, void, int& > );
   static_assert( !internal::has_unwind< hooks< void >, void, const int&, int& > );

   static_assert( !internal::has_apply< missing, void, nothing, int& > );
   static_assert( !internal::has_apply0< missing, void, nothing, int& > );
   static_assert( !internal::has_success< missing, void, int& > );
   static_assert( !internal::has_unwind< missing, void, int& > );

   struct lazy_eol
   {
      using eol_lazy_peek = void;
   };

   struct data_member
   {
      static constexpr int eol_lazy_peek = 0;
   };

   static_assert( internal::has_eol_lazy_peek< ascii::lazy::lf > );
   static_assert( !internal::has_eol_lazy_peek< ascii::scan::lf > );
   static_assert( internal::has_eol_lazy_peek< lazy_eol > );
   static_assert( !internal::has_eol_lazy_peek< data_member > );
   static_assert( !internal::has_eol_lazy_peek< missing > );

   struct pointer_message
   {
      static constexpr const char* error_message = "message";
   };

   struct array_message
   {
      static constexpr char error_message[] = "message";
   };

   struct mutable_message
   {
      inline static char error_message[] = "message";
   };

   struct view_message
   {
      static constexpr std::string_view error_message = "message";
   };

   // Error messages are checked after decay, rather than by convertibility.
   static_assert( internal::has_error_message< pointer_message > );
   static_assert( internal::has_error_message< array_message > );
   static_assert( !internal::has_error_message< mutable_message > );
   static_assert( internal::has_error_message< mutable_message, char* > );
   static_assert( !internal::has_error_message< view_message > );
   static_assert( !internal::has_error_message< missing > );
}  // namespace TAO_PEGTL_NAMESPACE::concept_tests

int main()
{
   return 0;
}
