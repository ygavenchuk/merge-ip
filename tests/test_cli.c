/*
 * Copyright 2024 Yurii Havenchuk.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "cli.h"

void test_parse_command_line_options(void **state) {
    char *args[] = {"merge-ip", "-f", "test.txt", "-d"};
    CommandLineOptions options = parse_command_line_options(4, args);

    assert_non_null(options.file);
    assert_string_equal(options.file, "test.txt");
    assert_true(options.debug);
}
