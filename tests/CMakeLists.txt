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

# Test sources
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.c")
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.c")

add_executable(
        merge-ip_tests
        test_main.c
        ${TEST_SOURCES}
        ${SOURCES}
        ${HEADERS}
)

target_include_directories(merge-ip_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

enable_cmocka(merge-ip_tests)
enable_strict_build_flags(merge-ip_tests)
enable_optimized_build_flags(merge-ip_tests)
enable_portable_math(merge-ip_tests)
enable_regex_fallback(merge-ip_tests)

# Enable CTest
include(CTest)
add_test(NAME merge-ip_tests COMMAND merge-ip_tests)
