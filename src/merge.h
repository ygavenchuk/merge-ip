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

#ifndef MERGE_IP_MERGE_H
#define MERGE_IP_MERGE_H

#include "ipRange.h"

#define CIDR_SIZE 20

typedef char CidrRecord[CIDR_SIZE];

/**
 * @brief Merges overlapping CIDR blocks
 *
 * This function processes a list of CIDR blocks, parses them into IP ranges,
 * merges overlapping or contiguous ranges, and then converts the merged ranges
 * back into CIDR strings. The resultant CIDR strings are stored in a newly
 * allocated array.
 *
 * @param cidr_list A list of C-strings representing CIDR blocks.
 *
 * @return The number of resulting CIDR records stored in the `cidr_records` array.
 */
ipRangeList merge_cidr(const ipRangeList *cidr_list);



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
size_t write_ip_ranges_to_file(const ipRangeList *ranges, FILE *out);


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
size_t print_ip_ranges(const ipRangeList *ranges);


#endif //MERGE_IP_MERGE_H
