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

void test_parse_command_line_options(void **state);
void test_empty_data_set(void **state);
void test_noise_data_set(void **state);
void test_merge_cidr_separated_by_new_line(void **state);
void test_merge_cidr_separated_by_space(void **state);
void test_merge_cidr_separated_by_tab(void **state);
void test_reading_buffer_captures_only_host_part_of_tailing_cidr(void **state);
void test_reading_buffer_captures_broken_part_of_tailing_cidr(void **state);
void test_reading_buffer_captures_only_part_of_tailing_cidr_prefix(void **state);

int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_parse_command_line_options),
            cmocka_unit_test(test_empty_data_set),
            cmocka_unit_test(test_noise_data_set),
            cmocka_unit_test(test_merge_cidr_separated_by_new_line),
            cmocka_unit_test(test_merge_cidr_separated_by_space),
            cmocka_unit_test(test_merge_cidr_separated_by_tab),
            cmocka_unit_test(test_reading_buffer_captures_only_host_part_of_tailing_cidr),
            cmocka_unit_test(test_reading_buffer_captures_broken_part_of_tailing_cidr),
            cmocka_unit_test(test_reading_buffer_captures_only_part_of_tailing_cidr_prefix),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
