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
size_t get_max_cidr_count(const ipRange *range) {
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
size_t summarize_address_range(const ipRange *range, CidrRecord **cidr_records) {
    uint32_t first = range->min_ip.s_addr;
    const uint32_t last = range->max_ip.s_addr;
    size_t count = 0;

    const size_t max_cidr_count = get_max_cidr_count(range);

    while (first <= last && count < max_cidr_count) {
        const unsigned int IP_BITS = 32;
        const uint32_t nbits = (uint32_t)fmin(
                count_right_hand_zero_bits(first, IP_BITS),
                bit_length(last - first + 1) - 1
        );

        const struct in_addr addr = {.s_addr = htonl(first)}; // return back to the network byte order

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
size_t to_cidr(const ipRangeList *ranges, CidrRecord **cidr_records) {
    // estimate maximum number of the CIDR records for memory allocation
    size_t max_cidr_count = 0;
    for (size_t i = 0; i < ranges->length; ++i) {
        max_cidr_count += get_max_cidr_count(&ranges->cidrs[i]);
    }

    *cidr_records = (CidrRecord *)malloc(max_cidr_count * sizeof(CidrRecord));
    if (*cidr_records == NULL) {
        fprintf(stderr, "ERROR: cannot allocate memory for the CIDR records\n");
        return 0;
    }

    size_t total_cidr_count = 0;
    for (size_t i = 0; i < ranges->length; ++i) {
        const size_t cidr_count = summarize_address_range(&ranges->cidrs[i], cidr_records);
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
 * @param cidr_records A pointer to an array of `CidrRecord` where the resultant CIDR strings
 *                     will be stored. Memory for this array will be allocated by the function.
 * @return The number of resulting CIDR records stored in the `cidr_records` array.
 */
size_t merge_cidr(ipRangeList *cidr_list, CidrRecord **cidr_records) {
    // Sort input CIDRs
    qsort(cidr_list->cidrs, cidr_list->length, sizeof(ipRange), compare_ip_ranges);

    ipRangeList mergedRanges = merge_ip_ranges(cidr_list);
    freeIpRangeList(cidr_list);
    const size_t cidr_count = to_cidr(&mergedRanges, cidr_records);
    freeIpRangeList(&mergedRanges);
    return cidr_count;
}
