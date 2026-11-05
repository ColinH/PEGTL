set(example_sources
  abnf_record.cpp
  abnf2pegtl.cpp
  analyze.cpp
  behavior.cpp
  calculator.cpp
  chomsky_hierarchy.cpp
  csv_builder.cpp
  csv_manual.cpp
  csv_static.cpp
  dispatch.cpp
  dynamic_match.cpp
  expression.cpp
  hello_world.cpp
  indent_aware.cpp
  iri_struct.cpp
  json_analyze.cpp
  json_builder.cpp
  json_classes.cpp
  json_count.cpp
  json_coverage.cpp
  json_parse.cpp
  json_parse_tree.cpp
  json_print_debug.cpp
  json_print_names.cpp
  json_record.cpp
  json_stream.cpp
  json_tokens.cpp
  json_trace.cpp
  lua53_analyze.cpp
  lua53_parse.cpp
  modulus_match.cpp
  open_metrics.cpp
  parse_tree.cpp
  parse_tree_user_state.cpp
  proto3_analyze.cpp
  proto3_parse.cpp
  recover.cpp
  s_expression.cpp
  semver2_parse.cpp
  sum.cpp
  symbol_table.cpp
  token_input_1.cpp
  token_input_2.cpp
  unescape.cpp
  uri_print_debug.cpp
  uri_print_names.cpp
  uri_struct.cpp
  uri_trace.cpp
)

# file(GLOB ...) is used to validate the above list of test_sources
file(GLOB glob_example_sources RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)

foreach(examplesourcefile ${example_sources})
  if(${examplesourcefile} IN_LIST glob_example_sources)
    list(REMOVE_ITEM glob_example_sources ${examplesourcefile})
  else()
    message(SEND_ERROR "File ${examplesourcefile} is missing from src/example")
  endif()

  get_filename_component(exename pegtl-example-${examplesourcefile} NAME_WE)
  add_executable(${exename} ${examplesourcefile})
  target_link_libraries(${exename} PRIVATE taocpp::pegtl)
  set_target_properties(${exename} PROPERTIES CXX_EXTENSIONS OFF)
  if(MSVC)
    target_compile_options(${exename} PRIVATE /W4 /WX /utf-8 /bigobj)
  else()
    target_compile_options(${exename} PRIVATE -pedantic -Wall -Wextra -Wshadow -Werror)
    if(MINGW)
      target_compile_options(${exename} PRIVATE -Wa,-mbig-obj)
    endif()
  endif()
endforeach()

if(glob_example_sources)
  foreach(ignored_source_file ${glob_example_sources})
    message(SEND_ERROR "File ${ignored_source_file} in src/example is ignored")
  endforeach()
endif()
