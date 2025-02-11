


file( GLOB_RECURSE ALL_CXX_SOURCE_FILES source/*.cpp include/*.h )

# clang-format 
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  add_custom_target( clang-format
                      COMMAND clang-format
                      -i
                      -style=file
                      ${ALL_CXX_SOURCE_FILES} )
endif()

# clang-tidy
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  SET( INCLUDE_ARGS "" )
  foreach(dir ${dirs})
    SET( INCLUDE_ARGS "${INCLUDE_ARGS} -I${dir}" )
  endforeach()
  add_custom_target( clang-tidy
                      COMMAND clang-tidy
                      ${ALL_CXX_SOURCE_FILES}
                      --
                      --system-headers=0 
                      ${INCLUDE_ARGS} )
endif()