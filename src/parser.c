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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"


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
    if (inet_pton(AF_INET, cidr, &ip) != 1) {
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
 * Checks if the given CIDR block has a full length (2 digits) prefix.
 *
 * @param cidr The CIDR block string to check.
 *
 * @return true  - the CIDR block has a full 2-digit prefix (e.g., "/32")
 *         false - otherwise
 */
bool has_full_prefix(const char* cidr) {
    const char *slash = strchr(cidr, '/');
    if (slash == NULL) {
        return false;
    }

    return (unsigned char)strlen(slash + 1) == 2;
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
size_t parse_content(const char *content, const regex_t *regex, ipRangeList *range_list, const bool require_full_cidr) {
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

        char cidr[CIDR_MAX_LENGTH] = {0};
        strncpy(cidr, content + cidr_start, cidr_length);

        const bool is_last_token = parsed_length >= content_length - CIDR_MIN_LENGTH;
        if (require_full_cidr && is_last_token && !has_full_prefix(cidr)) {
            parsed_length += cidr_start - token_end;
            break;
        }

        ensure_prefix(cidr);

        if (parse_cidr(cidr, ip_range) == 0) {
            appendIpRange(range_list, ip_range);
        }

        content += token_end;
    }

    free(ip_range);

    return parsed_length;
}
