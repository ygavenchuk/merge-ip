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

function(enable_optimized_build_flags target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target '${target}' does not exist.")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE
                    -g
                    -fno-omit-frame-pointer
                    -fsanitize=address
                    -fsanitize=integer
                    -fsanitize=leak
                    -fsanitize=memory
                    -fsanitize=pointer
                    -fsanitize=safestack
                    -fsanitize=undefined
            )
            target_link_options(${target} PRIVATE
                    -fsanitize=address
                    -fsanitize=integer
                    -fsanitize=leak
                    -fsanitize=memory
                    -fsanitize=pointer
                    -fsanitize=safestack
                    -fsanitize=undefined
            )
        endif()
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            # ==== Safety ====
            target_compile_options(${target} PRIVATE
                    -fstack-protector-strong
                    -fstack-clash-protection
                    -D_FORTIFY_SOURCE=2
            )

            # ==== Memory ====
            target_compile_options(${target} PRIVATE
                    -ffunction-sections
                    -fdata-sections
                    -fno-common
            )
            target_link_options(${target} PRIVATE
                    -Wl,
                    --gc-sections
            )

            # ==== Performance ====
            target_compile_options(${target} PRIVATE
                    -O3
                    -march=native
                    -mtune=native
            )
        elseif(MSVC)
            target_compile_options(${target} PRIVATE
                    /O2        # performance
                    /GL        # Whole Program Optimization
                    /Gw        # Global variable optimization
            )
            target_link_options(${target} PRIVATE
                    /LTCG      # Link Time Code Generation
                    /OPT:REF   # Remove unused code
                    /OPT:ICF   # Combining identical functions
            )
        endif()
    endif()

    # ==== Enabling LTO ====
    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endfunction()
