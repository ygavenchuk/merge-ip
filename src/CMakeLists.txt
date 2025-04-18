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

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if (CMAKE_BUILD_TYPE STREQUAL "Asan")
    add_definitions(
            -g
            -O1
            -Wno-unused-parameter
            -Wall
            -Werror
            -Wextra
            -fno-omit-frame-pointer
            -fsanitize=address
            -fsanitize=integer
            -fsanitize=undefined
    )
    add_link_options(-fsanitize=address -fsanitize=integer -fsanitize=undefined)
else ()
    add_definitions(
        -g
        -Wno-unused-parameter
        -Wall
        -Werror
        -Wextra
        -fno-omit-frame-pointer
    )
endif()

add_executable(merge-ip ${HEADERS} ${SOURCES})

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(log2 HAS_LOG2)
CHECK_FUNCTION_EXISTS(fmin HAS_FMIN)

if(NOT HAS_LOG2 OR NOT HAS_FMIN)
    target_link_libraries(merge-ip PRIVATE m)
endif()
