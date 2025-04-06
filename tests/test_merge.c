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

const char ALPHA_NUMERIC[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
// because SEPARATORS does not contain dots (`.`) nor colons (`:`) the `get_random_phrase()` function
// will never generate CIDR blocks with the correct format!
const char SEPARATORS[] = " \t\n\r\v\f-_()[]\\/;,%$@!&$%*+=~`\"'<>?";

typedef struct {
    const char **input_cidr_list;
    const size_t input_count;
    const char *expected_cidr_list;
    const size_t expected_count;
} TestCase;


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
            "172.31.1.0/24", "192.168.100.5", "192.168.100.105/255.255.255.255",
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

char *get_buffer(const size_t size) {
    char *buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, size);
    return buffer;
}

FILE *get_memory_stream(char *buffer, const size_t size) {
    FILE *stream = fmemopen(buffer, size, "w+");
    if (!stream) {
        perror("Failed to open memory stream");
        exit(EXIT_FAILURE);
    }

    return stream;
}


typedef struct TestDataStream {
    char *buffer;
    FILE *stream;
} TestDataStream;

void open_stream(TestDataStream* stream, const size_t length) {
    stream->buffer = get_buffer(length);
    stream->stream = get_memory_stream(stream->buffer, length);
}

void close_stream(const TestDataStream* stream) {
    fclose(stream->stream);
    free(stream->buffer);
}

void write_to_stream(const TestDataStream* stream, const TestCase* test_case, const char* separator) {
    for (size_t item = 0; item < test_case->input_count; item++) {
        fprintf(stream->stream, "%s%s", test_case->input_cidr_list[item], separator);
    }

    rewind(stream->stream);
}


size_t get_length(const TestCase *testCases, const char* separator) {
    size_t total_length = 1;  // an empty line at the EoF
    for (size_t item = 0; item < testCases->input_count; item++) {
        total_length += strlen(testCases->input_cidr_list[item]) + 1; // +1 for newline
    }

    return total_length + strlen(separator) * testCases->input_count;
}

typedef struct MergedTestCase {
    const size_t count;
    const char* result;
} MergedTestCase;

MergedTestCase merge_test_case(const TestCase* test_case, const char* separator) {
    const size_t max_buffer_length = get_length(test_case, separator);

    TestDataStream data_stream;
    open_stream(&data_stream, max_buffer_length);
    write_to_stream(&data_stream, test_case, separator);

    const ipRangeList *range_list = read_from_stream(data_stream.stream);
    close_stream(&data_stream);

    const ipRangeList *merged_ip_ranges = merge_cidr(range_list);

    TestDataStream result_stream;
    open_stream(&result_stream, max_buffer_length);

    const size_t count = write_ip_ranges_to_file(merged_ip_ranges, result_stream.stream);
    fclose(result_stream.stream);

    return (MergedTestCase){
        .count = count,
        .result = result_stream.buffer
    };
}


// this is not a "pure" test of the `merge_cidr()` function. It's rather a sort of integration test
// and, therefore it should be somewhere in `test_main.c` or so
void test_merge_cidr_separated_by_new_line(void **state) {
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const MergedTestCase merged_test_case = merge_test_case(&test_cases[i], "\n");

        assert_int_equal(merged_test_case.count, test_cases[i].expected_count);
        assert_string_equal(merged_test_case.result, test_cases[i].expected_cidr_list);

        free((void*)merged_test_case.result);
    }
}

void test_merge_cidr_separated_by_space(void **state) {
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const MergedTestCase merged_test_case = merge_test_case(&test_cases[i], " ");

        assert_int_equal(merged_test_case.count, test_cases[i].expected_count);
        assert_string_equal(merged_test_case.result, test_cases[i].expected_cidr_list);

        free((void*)merged_test_case.result);
    }
}

void test_merge_cidr_separated_by_tab(void **state) {
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const MergedTestCase merged_test_case = merge_test_case(&test_cases[i], "\t");

        assert_int_equal(merged_test_case.count, test_cases[i].expected_count);
        assert_string_equal(merged_test_case.result, test_cases[i].expected_cidr_list);

        free((void*)merged_test_case.result);
    }
}

void merge_cidr_separated_by_page(const size_t page_size) {
    char empty_page[page_size];
    memset(empty_page, ' ', page_size - 1);
    empty_page[page_size - 1] = '\0';

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const MergedTestCase merged_test_case = merge_test_case(&test_cases[i], empty_page);

        assert_int_equal(merged_test_case.count, test_cases[i].expected_count);
        assert_string_equal(merged_test_case.result, test_cases[i].expected_cidr_list);

        free((void*)merged_test_case.result);
    }
}

void test_empty_data_set(void **state) {
    const size_t page_size = 1200;

    char empty_page[page_size];
    memset(empty_page, ' ', page_size - 1);
    empty_page[page_size - 1] = '\0';

    const size_t max_buffer_length = page_size + 1;

    TestDataStream data_stream;
    open_stream(&data_stream, max_buffer_length);
    fprintf(data_stream.stream, "%s", empty_page);

    const ipRangeList *range_list = read_from_stream(data_stream.stream);
    close_stream(&data_stream);

    const ipRangeList *merged_ip_ranges = merge_cidr(range_list);

    TestDataStream result_stream;
    open_stream(&result_stream, max_buffer_length);

    const size_t count = write_ip_ranges_to_file(merged_ip_ranges, result_stream.stream);
    fclose(result_stream.stream);

    assert_int_equal(count, 0);
    assert_string_equal(result_stream.buffer, "");

    free((void*)result_stream.buffer);
}

char* get_random_word(const size_t length) {
    char *word = malloc(length + 1);
    if (!word) {
        fprintf(stderr, "Failed to allocate memory for random word\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < length; i++) {
        word[i] = ALPHA_NUMERIC[rand() % (sizeof(ALPHA_NUMERIC) - 1)];
    }
    word[length] = '\0';

    return word;
}

char* get_random_text(const size_t length) {
    char *phrase = malloc(length + 1);
    if (!phrase) {
        fprintf(stderr, "Failed to allocate memory for random text\n");
        exit(EXIT_FAILURE);
    }

    size_t current_length = 0;
    while (current_length < length) {
        const size_t word_length = rand() % (length - current_length) + 1;
        const char *word = get_random_word(word_length);

        for (size_t i = 0; i < word_length && current_length < length; i++) {
            phrase[current_length++] = word[i];
        }

        if (current_length < length) {
            phrase[current_length++] = SEPARATORS[ rand() % (sizeof(SEPARATORS) - 1) ];
        }

        free((char*)word);
    }

    phrase[length] = '\0';
    return phrase;
}

void test_noise_data_set(void **state) {
    const size_t page_size = 4242;
    const char* noise_data = get_random_text(page_size);
    const size_t max_buffer_length = page_size + 1;

    TestDataStream data_stream;
    open_stream(&data_stream, max_buffer_length);
    fprintf(data_stream.stream, "%s", noise_data);
    free((void*)noise_data);

    const ipRangeList *range_list = read_from_stream(data_stream.stream);
    close_stream(&data_stream);

    const ipRangeList *merged_ip_ranges = merge_cidr(range_list);

    TestDataStream result_stream;
    open_stream(&result_stream, max_buffer_length);

    const size_t count = write_ip_ranges_to_file(merged_ip_ranges, result_stream.stream);
    fclose(result_stream.stream);

    assert_int_equal(count, 0);
    assert_string_equal(result_stream.buffer, "");

    free((void*)result_stream.buffer);
}


void test_reading_buffer_captures_only_host_part_of_tailing_cidr(void **state) {
    // attempt to emulate case when due to buffer size a reading loop
    // cuts only host part of the last CIDR block, while there's a prefix
    //
    // assuming buffer size is `1024`, we need to put 999 symbols as a
    // separator between CIDRs
    // BUFFER_SIZE - strlen("192.168.0.0/24") + strlen("192.168.1.0") == 999
    merge_cidr_separated_by_page(999);
}

void test_reading_buffer_captures_broken_part_of_tailing_cidr(void **state) {
    // i.e. something like "192.168.0." or "192.168.1.0/" - those values
    // for sure cannot be considered as valid CIDRs
    merge_cidr_separated_by_page(1000); // 192.168.1.
    merge_cidr_separated_by_page(998);  // 192.168.1.0/
}

void test_reading_buffer_captures_only_part_of_tailing_cidr_prefix(void **state) {
    // attempt to emulate case when due to buffer size a reading loop
    // cuts only part of the prefix in the last CIDR block, in other words
    // the prefix contains 2 digits, but buffer caught only first one
    //
    // assuming buffer size is `1024`, we need to put 1001 symbols as a
    // separator between CIDRs
    // BUFFER_SIZE - strlen("192.168.0.0/24") + strlen("192.168.1.0/2") == 999
    merge_cidr_separated_by_page(997);
}
