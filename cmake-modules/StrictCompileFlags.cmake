#
# Copyright 2025 Yurii Havenchuk.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(enable_strict_build_flags target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target '${target_name}' not found.")
    endif()

    set(strict_compile_flags "")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Using GCC warnings")
        set(strict_compile_flags
                -Wall
                -Wextra
                -Werror
                -Wpedantic
                -Wshadow
                -Wconversion
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
                -fanalyzer
        )

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Using Clang warnings")
        set(strict_compile_flags
                -Wall
                -Wextra
                -Werror
                -Wpedantic
                -Wshadow
                -Wconversion
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
                -Weverything
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-disabled-macro-expansion
        )

    elseif(MSVC)
        message(STATUS "Using MSVC warnings")
        set(strict_compile_flags
                /W4
                /permissive-
                /sdl
        )
    endif()

    target_compile_options(${target_name} PRIVATE ${strict_compile_flags})
endfunction()
