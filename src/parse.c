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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "parse.h"

#define BUFFER_SIZE 1024


/**
 * @brief Initializes a ParsedData structure.
 *
 * This function allocates memory for the CIDR buffer within a ParsedData structure
 * and initializes its length to zero.
 *
 * @return A ParsedData structure with an allocated CIDR buffer and zero length.
 *
 * @note If memory allocation fails, the function prints an error message and exits the program.
 */
ParsedData initial_parsed_data() {
    ParsedData data;
    data.cidrs = malloc(sizeof(char *) * BUFFER_SIZE);
    if (!data.cidrs) {
        perror("Failed to allocate CIDR buffer");
        exit(EXIT_FAILURE);
    }
    data.length = 0;
    return data;
}

/**
 * Frees the memory allocated for the ParsedData structure.
 *
 * This function releases the memory used by the CIDR blocks
 * stored in the `cidrs` array of the `ParsedData` structure.
 *
 * @param data Pointer to ParsedData structure whose memory
 *             needs to be freed.
 */
void free_parsed_data(ParsedData *data) {
    for (size_t i = 0; i < data->length; ++i) {
        free((char*)data->cidrs[i]);    // casting to `char *` here is to remove compile time warning
    }
    free(data->cidrs);
}

/**
 * Adds a CIDR block to the ParsedData structure.
 *
 * This function ensures that the ParsedData structure has enough space
 * to store the new CIDR block, reallocating if necessary. It then duplicates
 * and stores the CIDR block and increments the length counter.
 *
 * @param data Pointer to the ParsedData structure.
 * @param cidr The CIDR block to add.
 *
 * @note If memory allocation fails, the function prints an error message and
 *       exits the program.
 */
void add_cidr(ParsedData *data, const char *cidr) {
    if (data->length >= BUFFER_SIZE) {
        data->cidrs = realloc(data->cidrs, 2 * data->length * sizeof(char *));
        if (!data->cidrs) {
            perror("Failed to reallocate CIDR buffer");
            exit(EXIT_FAILURE);
        }
    }
    data->cidrs[data->length] = strdup(cidr);
    if (!data->cidrs[data->length]) {
        perror("Failed to allocate CIDR");
        exit(EXIT_FAILURE);
    }
    data->length++;
}


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
 *
 * @return A ParsedData structure containing the extracted CIDR blocks and their count.
 *
 * @note If memory allocation fails, the function prints an error message and
 *       exits the program.
 */
ParsedData parse_content(char *content, regex_t *regex) {
    ParsedData data = initial_parsed_data();

    regmatch_t matches[2];

    while (regexec(regex, content, 2, matches, 0) == 0) {
        long long start = matches[1].rm_so;
        long long end = matches[1].rm_eo;
        long long length = end - start;

        char *cidr = malloc(length + 1);
        if (!cidr) {
            perror("Failed to allocate CIDR");
            exit(EXIT_FAILURE);
        }
        strncpy(cidr, content + start, length);
        cidr[length] = '\0';

        // Add `/32` prefix for "pure" IPv4
        if (strchr(cidr, '/') == NULL) {
            char *cidr_with_prefix = malloc(length + 4); // 3 for "/32" і 1 for '\0'
            if (!cidr_with_prefix) {
                perror("Failed to allocate CIDR with prefix");
                exit(EXIT_FAILURE);
            }
            sprintf(cidr_with_prefix, "%s/32", cidr);
            free(cidr);
            cidr = cidr_with_prefix;
        }

        add_cidr(&data, cidr);
        free(cidr);

        content += end;
    }

    return data;
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
ParsedData read_from_stream(FILE *stream) {
    ParsedData overall_data = initial_parsed_data();
    char buffer[BUFFER_SIZE];
    char *remaining = NULL;
    size_t remaining_size = 0;

    const char *pattern = "(([0-9]{1,3}\\.){3}[0-9]{1,3}(/([0-9]{1,2}))?)";
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        fprintf(stderr, "Failed to compile regex.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), stream)) {
        size_t buffer_len = strlen(buffer);

        // Allocate memory for the temporary buffer
        char *temp_buffer = malloc((remaining_size + buffer_len + 1) * sizeof(char));
        if (remaining) {
            memcpy(temp_buffer, remaining, remaining_size);
        }
        memcpy(temp_buffer + remaining_size, buffer, buffer_len + 1);

        // Parse the content
        ParsedData part_data = parse_content(temp_buffer, &regex);
        for (size_t i = 0; i < part_data.length; ++i) {
            add_cidr(&overall_data, part_data.cidrs[i]);
            free((char*)part_data.cidrs[i]);    // casting to `char *` here is to remove compile time warning
        }
        free(part_data.cidrs);

        // Find remaining unparsed part
        char *last_token = temp_buffer + remaining_size + buffer_len;
        while (last_token > temp_buffer && !isspace(*last_token)) {
            last_token--;
        }
        remaining_size = temp_buffer + remaining_size + buffer_len - last_token;

        remaining = realloc(remaining, remaining_size + 1);
        memcpy(remaining, last_token, remaining_size);
        remaining[remaining_size] = '\0';

        free(temp_buffer);
    }

    if (remaining && remaining_size > 0) {
        ParsedData part_data = parse_content(remaining, &regex);
        for (size_t i = 0; i < part_data.length; ++i) {
            add_cidr(&overall_data, part_data.cidrs[i]);
            free((char*)part_data.cidrs[i]);    // casting to `char *` here is to remove compile time warning
        }
        free(part_data.cidrs);
        free(remaining);
    }

    regfree(&regex);

    return overall_data;
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
ParsedData read_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    ParsedData data = read_from_stream(file);
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
ParsedData read_from_stdin() {
    return read_from_stream(stdin);
}
