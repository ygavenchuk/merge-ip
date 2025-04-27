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
#include <string.h>

#include "reader.h"
#include "parser.h"


/**
 * @brief Moves the reminder part of the buffer to the start and zeroes out the rest.
 *
 * This function takes a buffer and a starting position. It moves the content of the buffer
 * starting from the given position to the beginning of the buffer and zeroes out the rest.
 *
 * @param buffer The buffer to be modified.
 * @param reminder_start_pos The starting position from which to move content.
 *
 * @return The length of the moved fragment.
 */
size_t move_reminder_to_start(char *buffer, const size_t reminder_start_pos) {
    size_t buffer_size = strlen(buffer);
    if (reminder_start_pos >= buffer_size) {
        // If start_pos is out of bounds, zero the entire buffer
        memset(buffer, 0, buffer_size);
        return 0;
    }
    // Calculate the length of the fragment to move
    size_t fragment_length = buffer_size - reminder_start_pos;

    // Move the fragment to the beginning of the buffer
    memmove(buffer, buffer + reminder_start_pos, fragment_length);

    // Zero out the rest of the buffer
    memset(buffer + fragment_length, 0, buffer_size - fragment_length);

    return fragment_length;
}


/**
 * @brief Reads data from a given stream, parses it to extract CIDR blocks,
 *        and returns a ParsedData structure containing all the extracted CIDR blocks.
 *
 * This function reads the input stream line by line, and for each line,
 * it attempts to match and extract CIDR blocks using a regular expression.
 * The extracted CIDR blocks are stored in a ParsedData structure, which is returned
 * upon completion of the function. It handles potential line splits by keeping
 * any remaining part of a line in a buffer till it is fully parsed in subsequent read operations.
 *
 * @param stream The input file stream to read data from.
 * @return ParsedData structure containing all the parsed CIDR blocks.
 *
 * @note If memory allocation fails, the function prints an error message and exits the program.
 */
ipRangeList *read_from_stream(FILE *stream) {
    ipRangeList *ip_range_list = getIpRangeList(MAX_BUFFER_CAPACITY);

    regex_t regex = get_regex();

    char buffer[BUFFER_SIZE] = {0};
    // `fread()` doesn't automatically add '\0' at the end of the buffer,
    // so we have to read 1 symbol less
    size_t reminder_size = 0;
    while ( fread(buffer + reminder_size, sizeof(char), BUFFER_SIZE - reminder_size - 1, stream) ) {
        const size_t parsed_chars = parse_content(buffer, &regex, ip_range_list, true);
        reminder_size = move_reminder_to_start(buffer, parsed_chars);
    }

    if (reminder_size) {
        parse_content(buffer + reminder_size, &regex, ip_range_list, false);
    }

    regfree(&regex);

    return ip_range_list;
}


/**
 * Reads the content of a file specified by 'filename' and parses its data.
 *
 * This function opens the file in read mode, processes its content using
 * 'read_from_stream', and then closes the file.
 *
 * @param filename The name of the file to be read.
 * @return ParsedData struct containing the parsed data from the file.
 *
 * @note If the file cannot be opened, the function prints an error message
 *       and exits the program with a failure status.
 */
ipRangeList *read_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    ipRangeList *data = read_from_stream(file);
    fclose(file);
    return data;
}


/**
 * Reads and parses data from standard input (stdin).
 *
 * This function utilizes the read_from_stream function to read data
 * from the standard input and parse it. It returns a ParsedData
 * structure containing the results.
 *
 * @return A ParsedData structure containing the parsed CIDR blocks and their
 *         count.
 */
ipRangeList* read_from_stdin() {
    return read_from_stream(stdin);
}
