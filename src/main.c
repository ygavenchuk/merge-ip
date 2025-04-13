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
#include <stdio.h>

#include "ipRange.h"
#include "merge.h"
#include "parse.h"
#include "cli.h"

/**
 * @brief Entry point of the program that processes command line options
 *        to read CIDR blocks either from a file or standard input,
 *        merges the CIDR blocks, and outputs the merged result.
 *
 * This function performs the following steps:
 * 1. Parses command line options to determine input source and debug mode.
 * 2. Reads CIDR blocks from the specified file or standard input.
 * 3. Merges the CIDR blocks.
 * 4. Outputs the merged CIDR blocks.
 * 5. Frees allocated memory.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * @return int Returns 0 on successful execution.
 */
int main(const int argc, char *argv[]) {
    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            fprintf(stderr, "WSAStartup failed\n");
            return 1;
        }
    #endif

    const CommandLineOptions options = parse_command_line_options(argc, argv);
    ipRangeList *ip_range_list = NULL;

    if (options.file) {
        if (options.debug) {
            printf("DEBUG: Reading from file: %s\n", options.file);
        }
        ip_range_list = read_from_file(options.file);
    } else {
        if (options.debug) {
            printf("DEBUG: Reading from stdin\n");
        }
        ip_range_list = read_from_stdin();
    }

    ipRangeList *merged_ip_range = merge_cidr(ip_range_list);
    freeIpRangeList(ip_range_list);
    const size_t total_merged_cidrs = print_ip_ranges(merged_ip_range);
    freeIpRangeList(merged_ip_range);
    if (total_merged_cidrs > 0) {
        if (options.debug) {
            printf("DEBUG: Merged IP ranges in the CIDR format (total: %zu)\n", total_merged_cidrs);
        }
    }

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}
