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

#ifndef IPRANGE_H
#define IPRANGE_H

#include <arpa/inet.h>

// A struct to store minimal and maximal IPs
typedef struct {
  struct in_addr min_ip;
  struct in_addr max_ip;
} ipRange;


// Struct to store CIDR blocks and its count
typedef struct {
    ipRange *cidrs;
    size_t length;
    size_t capacity;
} ipRangeList;


/**
 * @brief Initializes a ipRangeList structure.
 *
 * This function allocates memory for the CIDR buffer within a ipRangeList structure
 * and initializes its length to zero.
 *
 * @param size the size of the CIDR buffer to allocate.
 *
 * @return A ipRangeList structure with an allocated CIDR buffer and zero length.
 *
 * @note If memory allocation fails, the function prints an error message and exits the program.
 */
ipRangeList getIpRangeList(const size_t size);


/**
 * Frees the memory allocated for the ipRangeList structure.
 *
 * This function releases the memory used by the CIDR blocks
 * stored in the `cidrs` array of the `ipRangeList` structure.
 *
 * @param data Pointer to ipRangeList structure whose memory
 *             needs to be freed.
 */
void freeIpRangeList(ipRangeList *data);

/**
 * Appends a copy of the ipRange block to the end of the `ipRangeList`.
 *
 * This function ensures that the `ipRangeList` structure has enough space
 * to store the new `ipRange` block, reallocating if necessary. It then duplicates
 * and stores the ipRange block and increments the length counter.
 *
 * @param data Pointer to the ipRangeList structure.
 * @param range The ipRange block to add.
 *
 * @note If memory allocation fails, the function prints an error message and
 *       exits the program.
 */
void appendIpRange(ipRangeList *data, const ipRange *range);

#endif //IPRANGE_H
