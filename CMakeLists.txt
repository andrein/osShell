project(osshell)

cmake_minimum_required(VERSION 2.8)


find_package(BISON)
find_package(FLEX)

flex_target(osScanner lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.c)
bison_target(osParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
add_flex_bison_dependency(osScanner osParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(osshell
  main.c
  args.c
  internals.c
  dirstack.c
  runner.c 
  ${BISON_osParser_OUTPUTS}
  ${FLEX_osScanner_OUTPUTS}
)

target_link_libraries(osshell readline)


