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
int main(int argc, char *argv[]) {
    CommandLineOptions options = parse_command_line_options(argc, argv);
    ParsedData parsed_data;

    if (options.file) {
        if (options.debug) {
            printf("DEBUG: Reading from file: %s\n", options.file);
        }
        parsed_data = read_from_file(options.file);
    } else {
        if (options.debug) {
            printf("DEBUG: Reading from stdin\n");
        }
        parsed_data = read_from_stdin();
    }

    CidrRecord *cidr_records = NULL;
    size_t cidr_count = merge_cidr(parsed_data.cidrs, parsed_data.length, &cidr_records);
    if (cidr_count > 0) {
        if (options.debug) {
            printf("DEBUG: Merged IP ranges in the CIDR format (total: %zu)\n", cidr_count);
        }

        for (size_t i = 0; i < cidr_count; ++i) {
            printf("%s\n", cidr_records[i]);
        }
        free(*cidr_records); // Free memory
    }

    free_parsed_data(&parsed_data);

    return 0;
}
