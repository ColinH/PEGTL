set(lint_sources
  actions.cpp
  controls.cpp
  debug.cpp
  extras.cpp
  inputs.cpp
  parse_tree.cpp
  rules.cpp
)

file(GLOB glob_lint_sources RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
foreach(source IN LISTS lint_sources)
  if(NOT source IN_LIST glob_lint_sources)
    message(SEND_ERROR "File ${source} is missing from src/lint")
  endif()
  list(REMOVE_ITEM glob_lint_sources ${source})

  get_filename_component(name ${source} NAME_WE)
  add_executable(pegtl-lint-${name} ${source})
  target_link_libraries(pegtl-lint-${name} PRIVATE taocpp::pegtl)
  set_target_properties(pegtl-lint-${name} PROPERTIES CXX_EXTENSIONS OFF)
  if(MSVC)
    target_compile_options(pegtl-lint-${name} PRIVATE /W4 /WX /utf-8 /bigobj)
  else()
    target_compile_options(pegtl-lint-${name} PRIVATE -pedantic -Wall -Wextra -Wshadow -Werror)
    if(MINGW)
      target_compile_options(pegtl-lint-${name} PRIVATE -Wa,-mbig-obj)
    endif()
  endif()
endforeach()

foreach(source IN LISTS glob_lint_sources)
  message(SEND_ERROR "File ${source} in src/lint is ignored")
endforeach()
