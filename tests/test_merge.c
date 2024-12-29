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

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include "merge.h"
#include "parse.h"
#include "ipRange.h"


typedef struct {
    const char **input_cidr_list;
    const size_t input_count;
    const char *expected_cidr_list;
    const size_t expected_count;
} TestCase;


char *get_buffer(const size_t size) {
    char *buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, size);
    return buffer;
}

FILE *get_stream(char *buffer, const size_t size) {
    FILE *stream = fmemopen(buffer, size, "w+");
    if (!stream) {
        perror("Failed to open memory stream");
        exit(EXIT_FAILURE);
    }

    return stream;
}


size_t get_length(const TestCase *testCases) {
    size_t total_length = 1;  // an empty line at the EoF
    for (size_t item = 0; item < testCases->input_count; item++) {
        total_length += strlen(testCases->input_cidr_list[item]) + 1; // +1 for newline
    }

    return total_length;
}


void write_to_stream(const TestCase* testCases, FILE *stream) {
    for (size_t item = 0; item < testCases->input_count; item++) {
        fprintf(stream, "%s\n", testCases->input_cidr_list[item]);
    }

    rewind(stream);
}


// this is not a "pure" test of the `merge_cidr()` function. It's rather a sort of integration test
// and, therefore it should be somewhere in `test_main.c` or so
void test_merge_cidr(void **state) {
    const TestCase test_cases[] = {
        {
            .input_cidr_list = (const char *[]){"192.168.0.0/24", "192.168.1.0/24"},
            .input_count = 2,
            .expected_cidr_list = "192.168.0.0/23\n",
            .expected_count = 1
        },
        {
            .input_cidr_list = (const char *[]){"10.0.0.0/8", "10.1.0.0/16", "10.0.2.0/24"},
            .input_count = 3,
            .expected_cidr_list = "10.0.0.0/8\n",
            .expected_count = 1
        },
        {
            .input_cidr_list = (const char *[]){"192.168.0.0/24", "192.168.2.0/24", "192.168.1.0/24"},
            .input_count = 3,
            .expected_cidr_list = "192.168.0.0/23\n"
                                  "192.168.2.0/24\n",
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
            .expected_cidr_list = "10.10.0.0/22\n"
                                  "10.10.4.0/24\n"
                                  "10.11.0.0/16\n"
                                  "172.16.0.0/12\n"
                                  "192.168.100.0/22\n"
                                  "192.168.104.0/22\n",
            .expected_count = 6
        },
        {
            .input_cidr_list = (const char *[]){
                "10.10.0.0/24",  "10.10.1.0/24",  "192.168.100.0/22", "10.10.2.0/24", "10.10.3.0/28",   "10.10.3.16/28",
                "10.10.3.32/28", "10.10.3.0/25",  "10.10.4.0/24",     "10.11.0.0/16", "10.10.3.128/25",
            },
            .input_count = 11,
            .expected_cidr_list = "10.10.0.0/22\n"
                                  "10.10.4.0/24\n"
                                  "10.11.0.0/16\n"
                                  "192.168.100.0/22\n",
            .expected_count = 4
        },
        {
            .input_cidr_list = (const char *[]){
                "10.10.0.0/24",  "10.10.1.0/24",  "192.168.100.0/22", "10.10.2.0/24", "10.10.3.0/28",   "10.10.3.16/28",
                "10.10.3.32/28",  "10.10.4.0/24", "10.11.0.0/16", "10.10.3.128/25",
            },
            .input_count = 10,
            .expected_cidr_list = "10.10.0.0/23\n"
                                  "10.10.2.0/24\n"
                                  "10.10.3.0/27\n"
                                  "10.10.3.32/28\n"
                                  "10.10.3.128/25\n"
                                  "10.10.4.0/24\n"
                                  "10.11.0.0/16\n"
                                  "192.168.100.0/22\n",
            .expected_count = 8
        }
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const size_t length = get_length(&test_cases[i]);
        char *test_cases_buffer = get_buffer(length);
        FILE *stream = get_stream(test_cases_buffer, length);
        write_to_stream(&test_cases[i], stream);
        ipRangeList range_list = read_from_stream(stream);
        fclose(stream);
        free(test_cases_buffer);

        const ipRangeList merged_ip_ranges = merge_cidr(&range_list);

        char *result_buffer = get_buffer(length);
        FILE *result_stream = get_stream(result_buffer, length);

        const size_t count = write_ip_ranges_to_file(&merged_ip_ranges, result_stream);
        fclose(result_stream);

        assert_int_equal(count, test_cases[i].expected_count);
        assert_string_equal(result_buffer, test_cases[i].expected_cidr_list);

        free(result_buffer);
    }
}
