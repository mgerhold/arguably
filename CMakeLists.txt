cmake_minimum_required(VERSION 3.21)
project(Arguably)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG c4ee726532178e556d923372f29163bd206d7732
)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850
)

FetchContent_MakeAvailable(fmt googletest)

if (MSVC)
    # enable code analysis
    set_property(GLOBAL PROPERTY VS_GLOBAL_EnableCppCoreCheck true)
    set_property(GLOBAL PROPERTY VS_GLOBAL_CodeAnalysisRuleSet CppCoreCheckRules.ruleset)
    set_property(GLOBAL PROPERTY VS_GLOBAL_RunCodeAnalysis true)

    # remove /W3 from defaults
    string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "-W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif ()

set(TARGET_LIST Arguably fmt gtest gtest_main)

add_library(Arguably STATIC include/arguably.hpp)

target_include_directories(Arguably PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(Arguably fmt)

# set warning levels
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("MSVC build")
    target_compile_options(Arguably PUBLIC /W4 /WX /permissive- /fsanitize=address)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("GCC build")
    target_compile_options(Arguably PUBLIC -Wall -Wextra -Werror -pedantic -Wconversion -pthread)
endif ()

# define DEBUG_BUILD
target_compile_definitions(Arguably PUBLIC "$<$<CONFIG:DEBUG>:DEBUG_BUILD>")

foreach (target ${TARGET_LIST})
    # static runtime library
    set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # set binary filenames
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_DEBUG ${target}-debug)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO ${target}-relwithdebinfo)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_RELEASE ${target}-release)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME_MINSIZEREL ${target}-minsizerel)

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message("Enabling LTO for target ${target}")
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message("Not enabling LTO for target ${target} (not a release build)")
    endif ()
endforeach ()

enable_testing()
add_subdirectory(test)