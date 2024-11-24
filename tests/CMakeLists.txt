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

# FetchContent to download CMocka
include(FetchContent)
FetchContent_Declare(
    cmocka
    GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
    GIT_TAG        cmocka-1.1.7
)
FetchContent_MakeAvailable(cmocka)

# Test sources
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.c")
add_executable(
        merge-ip_tests
        test_main.c
        ${TEST_SOURCES}
)

# Link original source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.c")

target_include_directories(merge-ip_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(merge-ip_tests PRIVATE cmocka::cmocka ${SOURCES})

add_test(NAME merge-ip_tests COMMAND merge-ip_tests)

# Additional test options
enable_testing()
add_custom_target(tests COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)
