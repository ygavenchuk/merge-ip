#
# Copyright 2024 Yurii Havenchuk.
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

cmake_minimum_required(VERSION 3.31)
project(merge-ip C)

set(CMAKE_C_STANDARD 11)

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(log2 HAS_LOG2)
CHECK_FUNCTION_EXISTS(fmin HAS_FMIN)

if(WIN32)
    find_package(PCRE QUIET)

    if (NOT PCRE_FOUND)
        message(STATUS "PCRE not found, downloading...")

        include(FetchContent)

        set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(PCRE2_BUILD_PCRE2POSIX ON CACHE BOOL "" FORCE)

        FetchContent_Declare(
                PCRE
                GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
                GIT_TAG pcre2-10.45
        )
        FetchContent_MakeAvailable(pcre)
    endif()
endif()


enable_testing()
add_subdirectory(src)
add_subdirectory(tests)

message(STATUS "CMake project setup complete. Ready to build and run tests.")
