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

#include "merge.h"


#define ALL_ONES 0xFFFFFFFF



// A struct to store minimal and maximal IPs
typedef struct {
    struct in_addr min_ip;
    struct in_addr max_ip;
} ipRange;


/**
 * @brief Computes the maximum number of CIDRs in a given IP range.
 *
 * This function calculates the difference between the maximum IP and
 * minimum IP in the given ipRange and adds one to include both endpoints.
 * The result represents the count of CIDRs that can exist within that range.
 *
 * @param range Pointer to an ipRange structure containing the minimum and
 * maximum IP addresses.
 * @return The maximum number of CIDRs within the specified IP range.
 */
size_t get_max_cidr_count(ipRange *range) {
    return range->max_ip.s_addr - range->min_ip.s_addr + 1;
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
    char cidr_copy[32];
    strncpy(cidr_copy, cidr, 32);

    // split CIDR to IP & mask
    char *slash = strchr(cidr_copy, '/');
    if (slash == NULL) {
        // if there's no prefix add /32
        strncat(cidr_copy, "/32", sizeof(cidr_copy) - strlen(cidr_copy) - 1);
        slash = strchr(cidr_copy, '/');
    }
    *slash = '\0';

    // convert IP address to binary format
    struct in_addr ip;
    if (inet_aton(cidr_copy, &ip) == 0) {
        fprintf(stderr, "ERROR: invalid IP address: %s\n", cidr_copy);
        return 2; // Error code
    }

    // compute mask
    int prefix_len = atoi(slash + 1);
    if (prefix_len < 0 || prefix_len > 32) {
        fprintf(stderr, "ERROR: invalid network mask: %s\n", slash + 1);
        return 3; // Код помилки
    }

    // compute minimal & maximal IP-address
    uint32_t mask = (uint32_t)htonl(~((1 << (32 - prefix_len)) - 1));
    range->min_ip.s_addr = ntohl(ip.s_addr & mask);
    range->max_ip.s_addr = ntohl(ip.s_addr | ~mask);

    return 0; // success
}

/**
 * Processes a list of CIDR blocks and fills an array with successfully parsed ipRange structures.
 *
 * This function takes a list of CIDR blocks, processes them using the parse_cidr function, and
 * stores the successfully parsed ranges in the specified result array. If a CIDR block fails
 * to parse, an error message is printed and the function continues with the next block.
 *
 * @param cidr_list A list of C-strings representing CIDR blocks.
 * @param cidr_list_size The size of the CIDR list.
 * @param result_array An array of ipRange structures to store the parsed IP ranges.
 * @return The count of successfully parsed CIDR blocks.
 */
size_t parse_cidr_list(const char **cidr_list, size_t cidr_list_size, ipRange *result_array) {
    size_t parsed_count = 0;

    for (size_t i = 0; i < cidr_list_size; ++i) {
        if (parse_cidr(cidr_list[i], &result_array[parsed_count]) == 0) {
            parsed_count++;
        } else {
            fprintf(stderr, "ERROR: cannot process CIDR: %s\n", cidr_list[i]);
        }
    }

    return parsed_count;
}

/**
 * @brief Compares two `ipRange` structures for sorting.
 *
 * This function is used to compare two `ipRange` structures, which contain
 * a minimum and a maximum IP address. The comparison is primarily based on
 * the `min_ip` field. If the `min_ip` values are equal, it compares the
 * `max_ip` values.
 *
 * @param a Pointer to the first `ipRange` structure.
 * @param b Pointer to the second `ipRange` structure.
 * @return An integer less than, equal to, or greater than zero if the `min_ip`
 *         of `a` is found, respectively, to be less than, equal to, or greater
 *         than the `min_ip` of `b`. If `min_ip` values are equal, the comparison
 *         is based on the `max_ip` values.
 */
int compare_ip_ranges(const void *a, const void *b) {
    ipRange *rangeA = (ipRange *)a;
    ipRange *rangeB = (ipRange *)b;

    // first compare min_ip
    if (rangeA->min_ip.s_addr < rangeB->min_ip.s_addr) return -1;
    if (rangeA->min_ip.s_addr > rangeB->min_ip.s_addr) return 1;

    // now - max_ip
    if (rangeA->max_ip.s_addr < rangeB->max_ip.s_addr) return -1;
    if (rangeA->max_ip.s_addr > rangeB->max_ip.s_addr) return 1;

    return 0; // they are equal
}


/**
 * Function to merge an array of IP ranges.
 *
 * This function takes an input array of `ipRange` structures representing IP ranges
 * and merges overlapping or contiguous ranges into a result array.
 *
 * @param ranges An array of `ipRange` structures representing the IP ranges to be merged.
 * @param count The number of elements in the `ranges` array.
 * @param result An array of `ipRange` structures where the merged result will be stored.
 * @return The number of merged IP ranges stored in the `result` array.
 */
size_t merge_ip_ranges(ipRange *ranges, size_t count, ipRange *result) {
    if (count == 0) {
        return 0;
    }

    size_t result_count = 0;
    ipRange current = ranges[0];

    if (current.max_ip.s_addr == ALL_ONES) {
        result[result_count++] = current;
        return result_count;
    }

    for (size_t i = 1; i < count; ++i) {
        if (ranges[i].max_ip.s_addr == ALL_ONES) {
            result[result_count++] = (ipRange){.min_ip = current.min_ip, .max_ip = {ALL_ONES}};
            return result_count;
        }

        if ((current.max_ip.s_addr + 1) >= ranges[i].min_ip.s_addr) {
            if (current.max_ip.s_addr < ranges[i].max_ip.s_addr) {
                current.max_ip = ranges[i].max_ip;
            }
        } else {
            result[result_count++] = current;
            current = ranges[i];
        }
    }

    if (result_count) {
        if (result[result_count - 1].max_ip.s_addr != current.max_ip.s_addr) {
            result[result_count++] = current;
        }
    } else {
        result[result_count++] = current;
    }

    return result_count;
}


/**
 * @brief Calculate the number of bits in the length of a range.
 *
 * This function computes the bit length of the given integer, effectively
 * counting the number of bits required to represent the value.
 *
 * @param x The integer value for which to calculate the bit length.
 * @return The number of bits required to represent the range.
 */
uint32_t bit_length(uint32_t x) {
    return x == 0 ? 0 : (int)log2(x) + 1;
}


/**
 * @brief Count the number of trailing zero bits in an integer.
 *
 * This function calculates the number of consecutive zero bits at the
 * least significant end of the given integer. If the input number is zero,
 * it returns the provided bit count.
 *
 * @param number The integer value to be analyzed.
 * @param bits The number of bits to consider for the operation.
 * @return The count of trailing zero bits.
 */
uint32_t count_righthand_zero_bits(uint32_t number, uint32_t bits) {
    if (number == 0) {
        return bits;
    }
    return (uint32_t)fmin(bits, bit_length(~number & (number - 1)));
}


/**
 * @brief Summarizes an IP address range into a list of CIDR notations.
 *
 * This function takes a range of IP addresses, identifies the largest possible
 * CIDR blocks within that range, and outputs the corresponding CIDR notation
 * strings. It returns the number of CIDR blocks created.
 *
 * @param range Structure containing the minimum and maximum IP addresses of the range.
 * @param cidr_records Pointer to an array of CidrRecord where CIDR notations will be stored.
 * @return The number of CIDR blocks in the summarized range.
 */
size_t summarize_address_range(ipRange range, CidrRecord **cidr_records) {
    uint32_t first = range.min_ip.s_addr;
    uint32_t last = range.max_ip.s_addr;
    size_t count = 0;
    const unsigned int IP_BITS = 32;

    size_t max_cidr_count = get_max_cidr_count(&range);

    while (first <= last && count < max_cidr_count) {
        uint32_t nbits = (uint32_t)fmin(
                count_righthand_zero_bits(first, IP_BITS),
                bit_length(last - first + 1) - 1
        );

        struct in_addr addr;
        addr.s_addr = htonl(first);  // return back to the network byte order

        snprintf((*cidr_records)[count], CIDR_SIZE, "%s/%d", inet_ntoa(addr), IP_BITS - nbits);
        count++;

        first += 1 << nbits;

        if (first - 1 == ALL_ONES) {
            break;
        }
    }

    return count;
}

/**
 * @brief Converts IP ranges into CIDR (Classless Inter-Domain Routing) notations.
 *
 * This function takes an array of IP ranges and converts each range into a list of
 * corresponding CIDR notations. It outputs the list of CIDR notations and returns
 * the total count of CIDR records created.
 *
 * @param ranges Pointer to an array of ipRange structures representing the IP ranges
 * to be converted.
 * @param count Number of IP ranges in the array.
 * @param cidr_records Output parameter which will point to an allocated array of generated
 * CIDR records.
 * @return The total number of CIDR records created; returns 0 if memory allocation fails.
 */
size_t to_cidr(ipRange *ranges, size_t count, CidrRecord **cidr_records) {
    // estimate maximum number of the CIDR records for memory allocation
    size_t max_cidr_count = 0;
    for (size_t i = 0; i < count; ++i) {
        max_cidr_count += get_max_cidr_count(&ranges[i]);
    }

    *cidr_records = (CidrRecord *)malloc(max_cidr_count * sizeof(CidrRecord));
    if (*cidr_records == NULL) {
        fprintf(stderr, "ERROR: cannot allocate memory for the CIDR records\n");
        return 0;
    }

    size_t total_cidr_count = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t cidr_count = summarize_address_range(ranges[i], cidr_records);
        if (cidr_count == 0) {
            fprintf(stderr, "ERROR: cannot allocate memory. Stop processing\n");
            free(*cidr_records - total_cidr_count); // free already allocated memory
            return 0;
        }
        *cidr_records += cidr_count; // move pointer to the next CIDR records
        total_cidr_count += cidr_count;
    }

    *cidr_records -= total_cidr_count; // move pointer to the beginning

    if (total_cidr_count < max_cidr_count) {
        *cidr_records = realloc(*cidr_records, total_cidr_count * sizeof(CidrRecord));
        if (*cidr_records == NULL) {
            fprintf(stderr, "ERROR: cannot allocate memory\n");
            return 0;
        }
    }

    return total_cidr_count;
}

/**
 * @brief Merges and converts a list of CIDR blocks into an array of `CidrRecord` structures.
 *
 * This function processes a list of CIDR blocks, parses them into IP ranges, merges overlapping
 * or contiguous ranges, and then converts the merged ranges back into CIDR strings. The resultant
 * CIDR strings are stored in a newly allocated array.
 *
 * @param cidr_list A list of C-strings representing CIDR blocks.
 * @param cidr_list_size The number of elements in the `cidr_list` array.
 * @param cidr_records A pointer to an array of `CidrRecord` where the resultant CIDR strings
 *                     will be stored. Memory for this array will be allocated by the function.
 * @return The number of resulting CIDR records stored in the `cidr_records` array.
 */
size_t merge_cidr(const char **cidr_list, size_t cidr_list_size, CidrRecord **cidr_records) {
    // Memory allocation for the array of results
    ipRange *ranges = malloc(cidr_list_size * sizeof(ipRange));

    if (ranges == NULL) {
        fprintf(stderr, "ERROR: cannot allocate memory\n");
        return 0;
    }

    size_t parsed_count = parse_cidr_list(cidr_list, cidr_list_size, ranges);

    // Sort result array
    qsort(ranges, parsed_count, sizeof(ipRange), compare_ip_ranges);
    // Allocating memory for the array of merged ranges
    ipRange *merged_ranges = malloc(parsed_count * sizeof(ipRange));
    if (merged_ranges == NULL) {
        fprintf(stderr, "ERROR: cannot allocate memory\n");
        free(ranges);
        return 0;
    }

    size_t merged_count = merge_ip_ranges(ranges, parsed_count, merged_ranges);
    free(ranges);  // Memory freeing

    size_t cidr_count = to_cidr(merged_ranges, merged_count, cidr_records);
    free(merged_ranges);  // Memory freeing
    return cidr_count;
}

