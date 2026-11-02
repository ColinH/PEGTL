// Copyright (c) 2020-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <tao/pegtl/action_input.hpp>
#include <tao/pegtl/apply_mode.hpp>
#include <tao/pegtl/argv_input.hpp>
#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/base_input.hpp>
#include <tao/pegtl/config.hpp>
#include <tao/pegtl/copy_input.hpp>
#include <tao/pegtl/count_position.hpp>
#include <tao/pegtl/default_eol.hpp>
#include <tao/pegtl/demangle.hpp>
#include <tao/pegtl/eol_exclude_tag.hpp>
#include <tao/pegtl/eol_matched_tag.hpp>
#include <tao/pegtl/eol_unknown_tag.hpp>
#include <tao/pegtl/file_input.hpp>
#include <tao/pegtl/inputs.hpp>
#include <tao/pegtl/match.hpp>
#include <tao/pegtl/member.hpp>
#include <tao/pegtl/mmap_input.hpp>
#include <tao/pegtl/normal.hpp>
#include <tao/pegtl/nothing.hpp>
#include <tao/pegtl/parse.hpp>
#include <tao/pegtl/pointer_position.hpp>
#include <tao/pegtl/position_with_source.hpp>
#include <tao/pegtl/read_input.hpp>
#include <tao/pegtl/rewind_mode.hpp>
#include <tao/pegtl/rules.hpp>
#include <tao/pegtl/stream.hpp>
#include <tao/pegtl/system.hpp>
#include <tao/pegtl/text_copy_input.hpp>
#include <tao/pegtl/text_file_input.hpp>
#include <tao/pegtl/text_mmap_input.hpp>
#include <tao/pegtl/text_position.hpp>
#include <tao/pegtl/text_position_with_source.hpp>
#include <tao/pegtl/text_read_input.hpp>
#include <tao/pegtl/text_view_input.hpp>
#include <tao/pegtl/type_list.hpp>
#include <tao/pegtl/version.hpp>
#include <tao/pegtl/view_input.hpp>

#if defined( __cpp_exceptions )
#include <tao/pegtl/parse_error.hpp>
#include <tao/pegtl/parse_error_base.hpp>
#include <tao/pegtl/parse_nested.hpp>
#endif

#include <tao/pegtl/action/add_guard.hpp>
#include <tao/pegtl/action/add_state.hpp>
#include <tao/pegtl/action/change_action.hpp>
#include <tao/pegtl/action/change_action_and_state.hpp>
#include <tao/pegtl/action/change_action_and_states.hpp>
#include <tao/pegtl/action/change_control.hpp>
#include <tao/pegtl/action/change_rule.hpp>
#include <tao/pegtl/action/change_state.hpp>
#include <tao/pegtl/action/change_states.hpp>
#include <tao/pegtl/action/check_consume.hpp>
#include <tao/pegtl/action/control_action.hpp>
#include <tao/pegtl/action/disable_action.hpp>
#include <tao/pegtl/action/enable_action.hpp>
#include <tao/pegtl/action/limit_consume.hpp>
#include <tao/pegtl/action/match_typed_state.hpp>
#include <tao/pegtl/action/require_apply.hpp>
#include <tao/pegtl/action/require_apply0.hpp>

#if defined ( __cpp_exceptions )
#include <tao/pegtl/action/build_for.hpp>
#include <tao/pegtl/action/build_to.hpp>
#include <tao/pegtl/action/builders.hpp>
#include <tao/pegtl/action/multi_traits.hpp>
#include <tao/pegtl/action/repeat_traits.hpp>
#include <tao/pegtl/action/value_traits.hpp>
#endif

#include <tao/pegtl/stream/alloc_buffer.hpp>
#include <tao/pegtl/stream/analyze_traits.hpp>
#include <tao/pegtl/stream/array_buffer.hpp>
#include <tao/pegtl/stream/buffers.hpp>
#include <tao/pegtl/stream/cstream_reader.hpp>
#include <tao/pegtl/stream/cstring_reader.hpp>
#include <tao/pegtl/stream/discard.hpp>
#include <tao/pegtl/stream/discard_input.hpp>
#include <tao/pegtl/stream/discard_input_on.hpp>
#include <tao/pegtl/stream/inputs.hpp>
#include <tao/pegtl/stream/is_stream.hpp>
#include <tao/pegtl/stream/istream_reader.hpp>
#include <tao/pegtl/stream/other_buffer.hpp>
#include <tao/pegtl/stream/prefetch.hpp>
#include <tao/pegtl/stream/require.hpp>
#include <tao/pegtl/stream/stream_input.hpp>
#include <tao/pegtl/stream/stream_input_base.hpp>
#include <tao/pegtl/stream/stream_input_with_source.hpp>
#include <tao/pegtl/stream/text_stream_input.hpp>
#include <tao/pegtl/stream/text_stream_input_with_source.hpp>

#include <tao/pegtl/control/apply_typed_state.hpp>
#include <tao/pegtl/control/input_control.hpp>
#include <tao/pegtl/control/remove_first_state.hpp>
#include <tao/pegtl/control/remove_first_states.hpp>
#include <tao/pegtl/control/remove_last_states.hpp>
#include <tao/pegtl/control/reverse_states.hpp>
#include <tao/pegtl/control/rewind_control.hpp>
#include <tao/pegtl/control/rewind_state_control.hpp>
#include <tao/pegtl/control/rotate_states_left.hpp>
#include <tao/pegtl/control/rotate_states_right.hpp>
#include <tao/pegtl/control/shuffle_states.hpp>
#include <tao/pegtl/control/state_control.hpp>

#if defined( __cpp_exceptions )
#include <tao/pegtl/control/must_if.hpp>
#endif

#include <tao/pegtl/debug/analyze.hpp>
#include <tao/pegtl/debug/analyze_traits.hpp>
#include <tao/pegtl/debug/coverage.hpp>
#include <tao/pegtl/debug/print.hpp>
#include <tao/pegtl/debug/print_coverage.hpp>
#include <tao/pegtl/debug/trace.hpp>
#include <tao/pegtl/debug/trace_action.hpp>
#include <tao/pegtl/debug/trace_state.hpp>
#include <tao/pegtl/debug/trace_struct.hpp>
#include <tao/pegtl/debug/trace_traits.hpp>
#include <tao/pegtl/debug/visit.hpp>

#if defined( TAO_PEGTL_TEST_ICU )
#include <tao/pegtl/unicode/icu16.hpp>
#include <tao/pegtl/unicode/icu32.hpp>
#include <tao/pegtl/unicode/icu8.hpp>
#endif

#include <tao/pegtl/unicode/utf16.hpp>
#include <tao/pegtl/unicode/utf32.hpp>
#include <tao/pegtl/unicode/utf8.hpp>

int main()
{
   static_assert( ' ' == 32 );
   static_assert( '0' == 48 );
   static_assert( 'a' == 97 );
   static_assert( 'Z' == 90 );
   static_assert( '~' == 126 );

   static_assert( sizeof( char ) == 1 );
   static_assert( sizeof( char16_t ) == 2 );
   static_assert( sizeof( char32_t ) == 4 );

   return 0;
}
