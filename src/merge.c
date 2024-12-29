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
#include <arpa/inet.h>
#include <math.h>

#include "merge.h"


#define ALL_ONES 0xFFFFFFFF


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
inline size_t get_max_cidr_count(const ipRange *range) {
    return range->max_ip.s_addr - range->min_ip.s_addr + 1;
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
    const ipRange *rangeA = (ipRange *)a;
    const ipRange *rangeB = (ipRange *)b;

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
 * @param rawRanges An array of `ipRange` structures representing the IP ranges to be merged.
 * @return The number of merged IP ranges stored in the `result` array.
 */
ipRangeList merge_ip_ranges(const ipRangeList *rawRanges) {
    if (rawRanges->length == 0) {
        return (ipRangeList){NULL, 0, 0};
    }

    ipRangeList result = getIpRangeList(rawRanges->length);
    ipRange current = rawRanges->cidrs[0];

    if (current.max_ip.s_addr == ALL_ONES) {
        appendIpRange(&result, &current);
        return result;
    }

    for (size_t i = 1; i < rawRanges->length; ++i) {
        if (rawRanges->cidrs[i].max_ip.s_addr == ALL_ONES) {
            appendIpRange(&result, &(ipRange){.min_ip = current.min_ip, .max_ip = {ALL_ONES}});
            return result;
        }

        if ((current.max_ip.s_addr + 1) >= rawRanges->cidrs[i].min_ip.s_addr) {
            if (current.max_ip.s_addr < rawRanges->cidrs[i].max_ip.s_addr) {
                current.max_ip = rawRanges->cidrs[i].max_ip;
            }
        } else {
            appendIpRange(&result, &current);
            current = rawRanges->cidrs[i];
        }
    }

    if (result.length) {
        if (result.cidrs[result.length - 1].max_ip.s_addr != current.max_ip.s_addr) {
            result.cidrs[result.length++] = current;
        }
    } else {
        result.cidrs[result.length++] = current;
    }

    return result;
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
uint32_t bit_length(const uint32_t x) {
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
uint32_t count_right_hand_zero_bits(const uint32_t number, const uint32_t bits) {
    if (number == 0) {
        return bits;
    }
    return (uint32_t)fmin(bits, bit_length(~number & (number - 1)));
}


/**
 * @brief Writes IP ranges in CIDR notation to a file.
 *
 * This function iterates over an array of `ipRange` structures, converts each range
 * into CIDR notation, and writes the CIDR blocks to the specified output file stream.
 * It returns the total number of CIDR blocks written.
 *
 * @param ranges Pointer to an array of `ipRange` structures representing the IP ranges to be written.
 * @param out The output file stream where the CIDR blocks will be written.
 *
 * @return The total number of CIDR blocks written to the file.
 */
size_t write_ip_ranges_to_file(const ipRangeList *ranges, FILE *out) {
    size_t total_cidr_count = 0;

    for (size_t i = 0; i < ranges->length; ++i) {
        uint32_t first = ranges->cidrs[i].min_ip.s_addr;
        const uint32_t last = ranges->cidrs[i].max_ip.s_addr;

        while ((first <= last) && (first - 1 != ALL_ONES)) {
            const unsigned short IP_BITS = 32;
            const uint32_t nbits = (uint32_t)fmin(
                count_right_hand_zero_bits(first, IP_BITS),
                bit_length(last - first + 1) - 1
            );

            const struct in_addr addr = {.s_addr = htonl(first)};
            fprintf(out, "%s/%d\n", inet_ntoa(addr), IP_BITS - nbits);
            total_cidr_count++;

            first += 1 << nbits;
        }

        if (first - 1 == ALL_ONES) {
            break;
        }
    }

    return total_cidr_count;
}

/**
 * @brief Prints IP ranges in CIDR notation to the standard output stream.
 *
 * This function is a wrapper around `write_ip_ranges_to_file` that prints the
 * CIDR blocks to the standard output stream.
 *
 * @param ranges Pointer to an array of `ipRange` structures representing the IP ranges to be printed.
 *
 * @return The total number of CIDR blocks printed.
 */
size_t print_ip_ranges(const ipRangeList *ranges) {
    return write_ip_ranges_to_file(ranges, stdout);
}

/**
 * @brief Merges and converts a list of CIDR blocks into an array of `CidrRecord` structures.
 *
 * This function processes a list of CIDR blocks, parses them into IP ranges, merges overlapping
 * or contiguous ranges, and then converts the merged ranges back into CIDR strings. The resultant
 * CIDR strings are stored in a newly allocated array.
 *
 * @param cidr_list A list of C-strings representing CIDR blocks.
 *
 * @return The number of resulting CIDR records stored in the `cidr_records` array.
 */
ipRangeList merge_cidr(const ipRangeList *cidr_list) {
    // Sort input CIDRs
    qsort(cidr_list->cidrs, cidr_list->length, sizeof(ipRange), compare_ip_ranges);
    return merge_ip_ranges(cidr_list);
}
