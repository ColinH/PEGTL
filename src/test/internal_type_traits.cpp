// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string_view>

#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/inputs.hpp>
#include <tao/pegtl/normal.hpp>
#include <tao/pegtl/nothing.hpp>
#include <tao/pegtl/unicode/utf8.hpp>

#include <tao/pegtl/action/control_action.hpp>
#include <tao/pegtl/extra/parse_tree.hpp>

#include <tao/pegtl/internal/has_apply.hpp>
#include <tao/pegtl/internal/has_apply0.hpp>
#include <tao/pegtl/internal/has_current_position.hpp>
#include <tao/pegtl/internal/has_eol_char_rule.hpp>
#include <tao/pegtl/internal/has_eol_lazy_peek.hpp>
#include <tao/pegtl/internal/has_eol_rule.hpp>
#include <tao/pegtl/internal/has_error_message.hpp>
#include <tao/pegtl/internal/has_match.hpp>
#include <tao/pegtl/internal/has_restart.hpp>
#include <tao/pegtl/internal/has_start.hpp>
#include <tao/pegtl/internal/has_success.hpp>
#include <tao/pegtl/internal/has_unwind.hpp>
#include <tao/pegtl/internal/is_simple_type.hpp>
#include <tao/pegtl/internal/rewind_guard.hpp>

#include <tao/pegtl/stream/has_direct_reader.hpp>

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

   static_assert( internal::has_eol_char_rule< ascii::scan::lf > );
   // static_assert( internal::has_eol_char_rule< ascii::scan::cr_crlf > );
   static_assert( !internal::has_eol_char_rule< ascii::lf > );
   static_assert( !internal::has_eol_char_rule< utf8::crlf > );

   static_assert( internal::has_eol_rule< internal::input_with_lines< ascii::any7, internal::view_input< char > > > );
   static_assert( !internal::has_eol_rule< internal::input_with_lines< void, internal::view_input< char > > > );
   static_assert( !internal::has_eol_rule< internal::view_input< char > > );

   static_assert( internal::has_start< argv_input<> > );
   static_assert( internal::has_start< view_input<> > );
   static_assert( internal::has_start< file_input<> > );
   static_assert( internal::has_start< read_input<> > );
   static_assert( internal::has_start< mmap_input<> > );

   static_assert( !internal::has_start< base_input<> > );

   namespace detection_tests
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

         template< typename Rule >
         Result unwind( int& ) &&;

         template< typename Rule, apply_mode A, rewind_mode M, template< typename... > class Action, template< typename... > class Control >
         static Result match( int&, int&& );

         template< typename Rule >
         using action = hooks;
      };

      // Return types must match exactly, including references.
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
      static_assert( parse_tree::internal::node_has_unwind< hooks< void >, void, void, int& > );
      static_assert( !parse_tree::internal::node_has_unwind< hooks< bool >, void, void, int& > );

      // Preserve the argument and receiver value categories used by detection.
      static_assert( !internal::has_apply< hooks< void >, void, nothing, int > );
      static_assert( !internal::has_apply0< hooks< void >, void, nothing, const int& > );
      static_assert( !internal::has_success< hooks< void >&, void, int& > );
      static_assert( !internal::has_unwind< hooks< void >, void, const int&, int& > );
      static_assert( !parse_tree::internal::node_has_unwind< hooks< void >&, void, void, int& > );

      // These two detectors intentionally accept any return type.
      static_assert( internal::has_match< void, apply_mode::enabled, rewind_mode::optional, hooks< void >::action, normal, int, int > );
      static_assert( internal::has_match< void, apply_mode::enabled, rewind_mode::optional, hooks< int >::action, normal, int, int > );
      static_assert( !internal::has_match< void, apply_mode::enabled, rewind_mode::optional, hooks< int >::action, normal, int, int& > );
      static_assert( internal::action_has_unwind< void, hooks< void >::action, int, int > );
      static_assert( internal::action_has_unwind< void, hooks< bool >::action, int, int > );
      static_assert( !internal::action_has_unwind< void, hooks< bool >::action, int, int& > );

      static_assert( !internal::has_apply< missing, void, nothing, int& > );
      static_assert( !internal::has_apply0< missing, void, nothing, int& > );
      static_assert( !internal::has_success< missing, void, int& > );
      static_assert( !internal::has_unwind< missing, void, int& > );
      static_assert( !internal::has_match< void, apply_mode::enabled, rewind_mode::optional, nothing, normal, int > );
      static_assert( !internal::action_has_unwind< void, nothing, int > );
      static_assert( !parse_tree::internal::node_has_unwind< missing, void, void, int& > );

      struct input
      {
         [[nodiscard]] int current_position() const;
         [[nodiscard]] int direct_reader() &&;
         int restart() &;
         int start() &&;
         int inc_rewind_guards() &&;

         using eol_char_rule = void;
         using eol_lazy_peek = void;
         using eol_rule = void;
      };

      static_assert( internal::has_current_position< const input& > );
      static_assert( internal::has_direct_reader< input > );
      static_assert( !internal::has_direct_reader< input& > );
      static_assert( internal::has_restart< input& > );
      static_assert( !internal::has_restart< input > );
      static_assert( internal::has_start< input > );
      static_assert( !internal::has_start< input& > );
      static_assert( internal::has_auto_discard< input > );
      static_assert( !internal::has_auto_discard< input& > );

      static_assert( !internal::has_current_position< missing > );
      static_assert( !internal::has_direct_reader< missing > );
      static_assert( !internal::has_restart< missing > );
      static_assert( !internal::has_start< missing > );
      static_assert( !internal::has_auto_discard< missing > );

      // Nested types can name void, but data members are not nested types.
      static_assert( internal::has_eol_char_rule< input > );
      static_assert( internal::has_eol_lazy_peek< input > );
      static_assert( internal::has_eol_rule< input > );
      static_assert( !internal::has_eol_char_rule< missing > );
      static_assert( !internal::has_eol_lazy_peek< missing > );
      static_assert( !internal::has_eol_rule< missing > );

      struct data_members
      {
         static constexpr int eol_char_rule = 0;
         static constexpr int eol_lazy_peek = 0;
         static constexpr int eol_rule = 0;
      };

      static_assert( !internal::has_eol_char_rule< data_members > );
      static_assert( !internal::has_eol_lazy_peek< data_members > );
      static_assert( !internal::has_eol_rule< data_members > );

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
   }  // namespace detection_tests

}  // namespace TAO_PEGTL_NAMESPACE

int main()
{
   return 0;
}
