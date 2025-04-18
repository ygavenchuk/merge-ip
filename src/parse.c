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

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "parse.h"

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
 * Parses a CIDR block and calculates the minimum and maximum IP addresses within the range.
 *
 * This function takes a CIDR block in the form "address/prefix_length", parses it, and computes
 * the range of IP addresses that fall within that CIDR block.
 *
 * @param cidr A C-string containing the CIDR notation (e.g., "192.168.1.0/24").
 * @param range A pointer to an ipRange struct where the computed min and max IP addresses will be stored.
 * @return Integer status code:
 *         - 0 on success
 *         - 2 if the IP address is invalid
 *         - 3 if the subnet mask is invalid
 */
int parse_cidr(const char *cidr, ipRange *range) {
    // split CIDR to IP & mask
    char *slash = strchr(cidr, '/');
    if (slash == NULL) {
        perror("CIDR without prefix! It should've never happened! I'm going to die now!");
        exit(EXIT_FAILURE);
    }
    *slash = '\0';

    // convert IP address to binary format
    struct in_addr ip;
    if (inet_aton(cidr, &ip) == 0) {
        fprintf(stderr, "ERROR: invalid IP address: %s\n", cidr);
        return 2; // Error code
    }

    // compute mask
    char *end = NULL;
    const long prefix_len = strtol(slash + 1, &end, 10);
    if (errno == ERANGE || prefix_len < 0 || prefix_len > CIDR_MAX_LENGTH) {
        fprintf(stderr, "ERROR: invalid network mask: %s\n", slash + 1);
        return 3; // Error code
    }

    // compute minimal & maximal IP-address
    const uint32_t mask = (uint32_t)htonl(~((1 << (CIDR_MAX_LENGTH - prefix_len)) - 1));
    range->min_ip.s_addr = ntohl(ip.s_addr & mask);
    range->max_ip.s_addr = ntohl(ip.s_addr | ~mask);

    return 0; // success
}


/**
 * @brief Checks if the given CIDR block is a pure IP, without prefix
 *
 * This function checks if the provided CIDR block string contains a prefix
 * (e.g., "/24"). If the CIDR block does not contain a prefix, it returns true.
 *
 * @param cidr The CIDR block string to check.
 *
 * @return true if the CIDR block does not have a prefix; false otherwise.
 */
bool is_host(const char* cidr) {
    return strchr(cidr, '/') == NULL;
}


/**
 * @brief Adds the "/32" prefix to the given CIDR block string, assuming it is a host.
 *
 * This function appends "/32" to the provided CIDR block string assuming it does not
 * already contain a prefix. The function modifies the original string in place.
 *
 * @param cidr The CIDR block string to which the prefix will be added.
 *
 */
void add_prefix(char* cidr) {
    const unsigned char length = (unsigned char)strlen(cidr); // max CIDR length is 32, which is less than 255

    cidr[length] = '/';
    cidr[length + 1] = '3';
    cidr[length + 2] = '2';
    cidr[length + 3] = 0;
}


/**
 * @brief Ensures that the given CIDR block string has a prefix.
 *
 * This function checks if the provided CIDR block string is a host (without a prefix).
 * If it is, it appends "/32" to the CIDR block string. The function modifies the original string in place.
 *
 * @param cidr The CIDR block string to check and potentially modify.
 */
void ensure_prefix(char* cidr) {
    if (is_host(cidr)) {
        add_prefix(cidr);
    }
}


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
 *
 * @return The number of characters parsed from the input string
 *
 * @note If memory allocation fails, the function prints an error message and
 *       exits the program.
 */
size_t parse_content(const char *content, const regex_t *regex, ipRangeList *range_list, bool ignore_tails) {
    regmatch_t matches[6];

    ipRange *ip_range = malloc(sizeof(ipRange));
    if (!ip_range) {
        perror("Failed to allocate IP range");
        exit(EXIT_FAILURE);
    }

    size_t cidr_end = 0;
    const size_t content_length = (size_t)strlen(content);
    size_t parsed_length = 0;
    while (regexec(regex, content, 6, matches, 0) == 0) {
        const size_t cidr_start = matches[1].rm_so;
        cidr_end = matches[1].rm_eo;
        const unsigned char cidr_length = (unsigned char)(cidr_end - cidr_start);
        const size_t token_end = matches[5].rm_eo; // position of the last matched token (CIDR + whitespaces)
        parsed_length += token_end;

        if (ignore_tails && parsed_length >= content_length - CIDR_MIN_LENGTH && !isspace(content[cidr_end])) {
            parsed_length += cidr_start - token_end;
            break;
        }

        char cidr[CIDR_MAX_LENGTH] = {0};
        strncpy(cidr, content + cidr_start, cidr_length);

        ensure_prefix(cidr);

        if (parse_cidr(cidr, ip_range) == 0) {
            appendIpRange(range_list, ip_range);
        }

        content += token_end;
    }

    free(ip_range);

    return parsed_length;
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

    // surprisingly the `regex_t` doesn't capture space symbols by the `\s`,
    // so I have to explicitly list them in the pattern
    const char *CIDR_PATTERN = "(([0-9]{1,3}\\.){3}[0-9]{1,3}(/([0-9]{1,2}))?)([ \t\n\r\v\f]*)";
    regex_t regex;
    if (regcomp(&regex, CIDR_PATTERN, REG_EXTENDED | REG_NEWLINE)) {
        fprintf(stderr, "Failed to compile regex.\n");
        exit(EXIT_FAILURE);
    }

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
inline ipRangeList *read_from_stdin() {
    return read_from_stream(stdin);
}
