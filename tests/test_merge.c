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

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "merge.h"

void test_merge_cidr(void **state) {
    struct TestCase {
        const char **input_cidr_list;
        size_t input_count;
        const char **expected_cidr_list;
        size_t expected_count;
    };

    struct TestCase test_cases[] = {
        {
            .input_cidr_list = (const char *[]){"192.168.0.0/24", "192.168.1.0/24"},
            .input_count = 2,
            .expected_cidr_list = (const char *[]){"192.168.0.0/23"},
            .expected_count = 1
        },
        {
            .input_cidr_list = (const char *[]){"10.0.0.0/8", "10.1.0.0/16", "10.0.2.0/24"},
            .input_count = 3,
            .expected_cidr_list = (const char *[]){"10.0.0.0/8"},
            .expected_count = 1
        },
        {
            .input_cidr_list = (const char *[]){"192.168.0.0/24", "192.168.2.0/24", "192.168.1.0/24"},
            .input_count = 3,
            .expected_cidr_list = (const char *[]){"192.168.0.0/23", "192.168.2.0/24"},
            .expected_count = 2
        },
        {
            .input_cidr_list = (const char *[]){
                "10.10.0.0/24",  "10.10.1.0/24",  "192.168.100.0/22", "10.10.2.0/24",     "10.10.3.0/28",
                "10.10.3.16/28", "10.10.3.32/28", "172.31.0.0/16",    "10.10.3.0/25",     "10.10.4.0/24",
                "10.11.0.0/16",  "172.31.1.1",    "10.10.3.128/25",   "192.168.104.0/22", "172.16.0.0/12",
                "172.31.1.0/24", "192.168.100.5",
            },
            .input_count = 17,
            .expected_cidr_list = (const char *[]){
                "10.10.0.0/22",
                "10.10.4.0/24",
                "10.11.0.0/16",
                "172.16.0.0/12",
                "192.168.100.0/22",
                "192.168.104.0/22",
            },
            .expected_count = 6
        },
        {
            .input_cidr_list = (const char *[]){
                "10.10.0.0/24",  "10.10.1.0/24",  "192.168.100.0/22", "10.10.2.0/24", "10.10.3.0/28",   "10.10.3.16/28",
                "10.10.3.32/28", "10.10.3.0/25",  "10.10.4.0/24",     "10.11.0.0/16", "10.10.3.128/25",
            },
            .input_count = 11,
            .expected_cidr_list = (const char *[]){
                "10.10.0.0/22",
                "10.10.4.0/24",
                "10.11.0.0/16",
                "192.168.100.0/22",
            },
            .expected_count = 4
        },
        {
            .input_cidr_list = (const char *[]){
                "10.10.0.0/24",  "10.10.1.0/24",  "192.168.100.0/22", "10.10.2.0/24", "10.10.3.0/28",   "10.10.3.16/28",
                "10.10.3.32/28",  "10.10.4.0/24", "10.11.0.0/16", "10.10.3.128/25",
            },
            .input_count = 10,
            .expected_cidr_list = (const char *[]){
                "10.10.0.0/23",
                "10.10.2.0/24",
                "10.10.3.0/27",
                "10.10.3.32/28",
                "10.10.3.128/25",
                "10.10.4.0/24",
                "10.11.0.0/16",
                "192.168.100.0/22",
            },
            .expected_count = 8
        }
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        CidrRecord *cidr_records = NULL;
        size_t count = merge_cidr(test_cases[i].input_cidr_list, test_cases[i].input_count, &cidr_records);

        assert_int_equal(count, test_cases[i].expected_count);

        for (size_t j = 0; j < test_cases[i].expected_count; j++) {
            assert_string_equal(cidr_records[j], test_cases[i].expected_cidr_list[j]);
        }

        free(cidr_records);
    }
}