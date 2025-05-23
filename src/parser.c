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
 * @brief Returns a precompiled regex_t object for matching CIDR blocks.
 *
 * This function compiles a regular expression that matches CIDR blocks in the format
 * "address/prefix_length". The regex is compiled with extended syntax and newline support.
 *
 * The caller is responsible for freeing the regex_t object using `regfree()` after use.
 *
 * @return A regex_t object representing the compiled regular expression.
 */
regex_t get_regex() {
    // surprisingly, the `regex_t` doesn't capture space symbols by the `\s`,
    // so I have to explicitly list them in the pattern
    const char *CIDR_PATTERN = "(([0-9]{1,3}\\.){3}[0-9]{1,3}(/([0-9]{1,2}))?)([ \t\n\r\v\f]*)";

    regex_t regex;
    if (regcomp(&regex, CIDR_PATTERN, REG_EXTENDED | REG_NEWLINE)) {
        fprintf(stderr, "Failed to compile regex.\n");
        exit(EXIT_FAILURE);
    }

    return regex;
}


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
 * @brief Checks if the CIDR block is potentially broken.
 *
 * This function checks if the CIDR block is potentially broken by looking
 * for a two-digit prefix at the end of the CIDR block. If the CIDR block
 * is broken, it returns true; otherwise, it returns false.
 *
 * @param content The content string to check.
 * @param cidr_end The end position of the CIDR block in the content string.
 *
 * @return true if the CIDR block is potentially broken; false otherwise.
 */
bool maybe_broken_cidr(const char* content, const size_t cidr_end) {
    if (isspace(content[cidr_end])) {
        return false; // the regex has caught the whole CIDR block
    }

    // There's no 2-digit prefix in the captured CIDR.
    // Theoretically, it might be in the next text not
    // read yet into the buffer, so the CIDR potentially MAY be broken
    return !(
        content[cidr_end - 3] == '/'
        && isdigit(content[cidr_end - 2])
        && isdigit(content[cidr_end - 1])
    );
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


typedef struct {
    bool maybe_broken;
    size_t start;
    size_t end;
    char cidr[CIDR_MAX_LENGTH];
} CIDRToken;


/** @brief Extracts a CIDR block from the given content using the provided regex.
 *
 * This function uses a regular expression to find and extract a CIDR block from
 * the input content. It fills the provided CIDRToken structure with the extracted
 * information, including the start and end positions of the match.
 *
 * @param content The input string to search for a CIDR block.
 * @param regex A precompiled regular expression to match CIDR blocks.
 * @param token A pointer to a CIDRToken structure to store the extracted information.
 *
 * @return true if a CIDR block was found and extracted; false otherwise.
 */
bool get_token(const char* content, const regex_t* regex, CIDRToken* token) {
    regmatch_t matches[6];
    memset(token->cidr, 0, CIDR_MAX_LENGTH);

    if (regexec(regex, content, 6, matches, 0) != 0) {
        return false;
    }

    token->start = matches[1].rm_so;
    const size_t cidr_end = matches[1].rm_eo;
    token->end = matches[5].rm_eo; // position of the last matched token (CIDR + whitespaces)
    token->maybe_broken = maybe_broken_cidr(content, cidr_end);

    strncpy(token->cidr, content + token->start, cidr_end - token->start);
    ensure_prefix(token->cidr);

    return true;
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
    ipRange *ip_range = malloc(sizeof(ipRange));
    if (!ip_range) {
        perror("Failed to allocate IP range");
        exit(EXIT_FAILURE);
    }

    const size_t content_length = (size_t)strlen(content);
    size_t parsed_length = 0;
    CIDRToken token;
    while ( get_token(content, regex, &token) ) {
        parsed_length += token.end;

        const bool is_last_token = parsed_length >= content_length - CIDR_MIN_LENGTH;
        if ( require_full_cidr && is_last_token && token.maybe_broken ) {
            parsed_length += token.start - token.end;
            break;
        }

        if (parse_cidr(token.cidr, ip_range) == 0) {
            appendIpRange(range_list, ip_range);
        }

        content += token.end;
    }

    free(ip_range);

    return parsed_length;
}
