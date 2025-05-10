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

function(enable_regex_fallback target)
    if(NOT WIN32)
        return()
    endif()

    find_package(PCRE QUIET)

    if (NOT PCRE_FOUND)
        message(STATUS "PCRE2 not found — fetching fallback POSIX-compatible regex support...")

        include(FetchContent)

        set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(PCRE2_BUILD_PCRE2POSIX ON CACHE BOOL "" FORCE)

        FetchContent_Declare(
                PCRE
                GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
                GIT_TAG        pcre2-10.45
        )
        FetchContent_MakeAvailable(PCRE)
    endif()

    target_link_libraries(${target} PRIVATE pcre2-posix ws2_32)
endfunction()
