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

#ifndef MERGE_IP_PARSE_H
#define MERGE_IP_PARSE_H

#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
    #include <stdint.h>
    #include <pcre2posix.h>
#else
    #include <regex.h>
#endif

#include "ipRange.h"


#define BUFFER_SIZE 1024
// strlen("255.255.255.255/255.255.255.255") + '\0'
#define CIDR_MAX_LENGTH 32  // 3 for "/32" and 1 for '\0'
// strlen("1.1.1.1")
#define CIDR_MIN_LENGTH 7
// The buffer may contain a limited number of records. In the limiting case there cannot be more than
// ceil(BUFFER_SIZE / (CIDR_MIN_LENGTH + 1) )
// Here:
//   - `(BUFFER_SIZE + (CIDR_MIN_LENGTH + 1) - 1)` is an integer equivalent of `ceil()`
//   - `+1` in the denominator is for a separator between adjacent records
#define MAX_BUFFER_CAPACITY ((BUFFER_SIZE + (CIDR_MIN_LENGTH + 1) - 1) / (CIDR_MIN_LENGTH + 1))


/**
 * @brief Returns a precompiled regex_t object for matching CIDR blocks.
 *
 * This function compiles a regular expression that matches CIDR blocks in the format
 * "address/prefix_length". The regex is compiled with extended syntax and newline support.
 *
 * The caller is responsible for freeing the regex_t object using `regfree()` after use.
 *
 * @return A regex_t object representing the compiled regular expression.
 */
regex_t get_regex();


/**
 * @brief Parses content for CIDR blocks defined by the given regular expression
 * and returns the parsed data.
 *
 * This function scans the input content for matches against the provided
 * regular expression to identify CIDR blocks. For IPv4 addresses without a
 * CIDR prefix, it appends `/32` by default. Each matched CIDR block is stored
 * in the ParsedData structure.
 *
 * @param content The input string to be parsed for CIDR blocks.
 * @param regex A precompiled regular expression to identify CIDR blocks.
 * @param range_list A pointer to the ipRangeList structure to store the extracted CIDR blocks.
 * @param require_full_cidr A boolean flag indicating whether to require a full CIDR block
 *
 * @return The number of characters parsed from the input string
 *
 * @note If memory allocation fails, the function prints an error message and
 *       exits the program.
 */
size_t parse_content(const char *content, const regex_t *regex, ipRangeList *range_list, const bool require_full_cidr);

#endif //MERGE_IP_PARSE_H
