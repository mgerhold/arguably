add_executable(
        Arguably_Tests
        tests.cpp
)

target_link_libraries(Arguably_Tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Arguably)

# set warning levels
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("MSVC build")
    target_compile_options(Arguably_Tests PUBLIC /W4 /permissive- /fsanitize=address)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("GCC build")
    target_compile_options(Arguably_Tests PUBLIC -Wall -Wextra -pedantic -Wconversion -pthread)
endif ()

# define DEBUG_BUILD
target_compile_definitions(Arguably_Tests PUBLIC "$<$<CONFIG:DEBUG>:DEBUG_BUILD>")

# static runtime library
set_property(TARGET Arguably_Tests PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# set binary filenames
set_target_properties(Arguably_Tests PROPERTIES OUTPUT_NAME_DEBUG Tests-debug)
set_target_properties(Arguably_Tests PROPERTIES OUTPUT_NAME_RELWITHDEBINFO Tests-relwithdebinfo)
set_target_properties(Arguably_Tests PROPERTIES OUTPUT_NAME_RELEASE Tests-release)
set_target_properties(Arguably_Tests PROPERTIES OUTPUT_NAME_MINSIZEREL Tests-minsizerel)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Enabling LTO for target Arguably_Tests")
    set_property(TARGET Arguably_Tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else ()
    message("Not enabling LTO for target Arguably_Tests (not a release build)")
endif ()

add_test(
        NAME unit
        COMMAND Arguably_Tests
)
