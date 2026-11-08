set(CMAKE_CXX_STANDARD 23)
include(CheckCXXSourceCompiles)
include(CMakePushCheckState)
cmake_push_check_state()
list(APPEND CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}/include")
# Availability can change when reconfiguring with a different deployment target.
unset(PEGTL_HAS_FLOATING_CHARCONV CACHE)
check_cxx_source_compiles("
  #include <tao/pegtl/extra/internal/charconv_features.hpp>
  #if !defined(TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS)
  #error Floating-point from_chars is unavailable
  #endif
  int main() { return 0; }
" PEGTL_HAS_FLOATING_CHARCONV)
cmake_pop_check_state()

set(compile_sources
  action_add_guard.cpp
  action_add_state.cpp
  action_apply_and_apply0.cpp
  action_change_action.cpp
  action_change_state.cpp
  action_nothing_apply.cpp
  action_nothing_apply_input.cpp
  ascii_aistring_prefix_min.cpp
  ascii_aistring_prefix_tail.cpp
  ascii_astring_prefix_min.cpp
  ascii_astring_prefix_tail.cpp
  control_remove_first_states.cpp
  control_remove_last_states.cpp
  extra_charconv_base_max.cpp
  extra_charconv_base_min.cpp
  extra_charconv_floating.cpp
  extra_charconv_floating_format.cpp
  extra_charconv_floating_state.cpp
  extra_charconv_floating_to_family.cpp
  extra_charconv_floating_to_mismatch.cpp
  extra_charconv_integral.cpp
  extra_charconv_to_integer.cpp
  extra_charconv_to_mismatch.cpp
  extra_charconv_to_other_rule.cpp
  extra_dispatch_multiple.cpp
  extra_parse_tree_selector.cpp
  extra_partial_apply1_missing.cpp
  extra_partial_apply1_missing0.cpp
  extra_partial_apply1_one.cpp
  extra_partial_apply1_zero.cpp
  extra_unescape_char.cpp
  extra_unescape_char_size.cpp
  extra_unescape_unicode_min.cpp
  input_copy_string_view.cpp
  input_source_mismatch.cpp
  input_stream_array_buffer.cpp
  input_stream_array_buffer_size.cpp
  input_text_void_eol.cpp
  input_view_data_width.cpp
  input_view_string_rvalue.cpp
  missing_apply.cpp
  parse_const_input.cpp
  parse_nested_const_input.cpp
  position_with_source.cpp
  rule_function_result.cpp
  rule_ione.cpp
  rule_keyword.cpp
  rule_not_ione.cpp
  rule_not_one.cpp
  rule_not_range.cpp
  rule_not_ranges.cpp
  rule_not_ranges_empty.cpp
  rule_one.cpp
  rule_range.cpp
  rule_ranges.cpp
  rule_ranges_empty.cpp
  rule_ranges_multiple.cpp
  rule_rep_min_max.cpp
  rule_utf8_string_surrogate.cpp
  rule_utf8_string_too_large.cpp
  type_list_contains_nested.cpp
  type_list_contains_type_list.cpp
)

# Validate the explicit list, as for the runtime tests.
file(GLOB glob_compile_sources RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
foreach(source IN LISTS compile_sources)
  if(NOT source IN_LIST glob_compile_sources)
    message(SEND_ERROR "File ${source} is missing from src/compile")
  endif()
  list(REMOVE_ITEM glob_compile_sources ${source})

  if(source MATCHES "^extra_charconv_floating" AND NOT PEGTL_HAS_FLOATING_CHARCONV)
    continue()
  endif()

  get_filename_component(name ${source} NAME_WE)
  foreach(mode IN ITEMS accept reject)
    set(target pegtl-compile-${mode}-${name})
    # CTest builds each mode separately; rejection targets must not enter ALL.
    add_library(${target} OBJECT ${source})
    set_target_properties(${target} PROPERTIES
      EXCLUDE_FROM_ALL ON
      CXX_EXTENSIONS OFF
    )
    target_link_libraries(${target} PRIVATE taocpp::pegtl)
    if(mode STREQUAL "accept")
      target_compile_definitions(${target} PRIVATE TAO_PEGTL_COMPILE_ACCEPT=1 TAO_PEGTL_COMPILE_REJECT=0)
    else()
      target_compile_definitions(${target} PRIVATE TAO_PEGTL_COMPILE_ACCEPT=0 TAO_PEGTL_COMPILE_REJECT=1)
      # Intentionally invalid sources should not be analyzed by clang-tidy.
      set_target_properties(${target} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    endif()
    if(MSVC)
      target_compile_options(${target} PRIVATE /W4 /WX /utf-8 /bigobj)
    else()
      target_compile_options(${target} PRIVATE -pedantic -Wall -Wextra -Wshadow -Werror)
      if(MINGW)
        target_compile_options(${target} PRIVATE -Wa,-mbig-obj)
      endif()
    endif()

    add_test(NAME ${target}
      COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config $<CONFIG> --target ${target})
    # Multiple build tools must not modify the same build tree concurrently.
    set_tests_properties(${target} PROPERTIES LABELS compile RESOURCE_LOCK pegtl_compile_build)
  endforeach()

  # Do not count a broken accept case as a successful rejection.
  set_tests_properties(pegtl-compile-accept-${name} PROPERTIES FIXTURES_SETUP compile_${name})
  set_tests_properties(pegtl-compile-reject-${name} PROPERTIES
    FIXTURES_REQUIRED compile_${name}
    WILL_FAIL TRUE
  )
endforeach()

foreach(source IN LISTS glob_compile_sources)
  message(SEND_ERROR "File ${source} in src/compile is ignored")
endforeach()

