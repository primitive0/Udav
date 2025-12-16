if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fmodules-reduced-bmi>)
endif()
