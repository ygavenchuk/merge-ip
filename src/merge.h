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
 * @param cidr_list_size The number of elements in the `cidr_list` array.
 * @param cidr_records A pointer to an array of `CidrRecord` where the resultant CIDR strings
 *                     will be stored. Memory for this array will be allocated by the function.
 *
 * @return The number of resulting CIDR records stored in the `cidr_records` array.
 */
size_t merge_cidr(const char **cidr_list, size_t cidr_list_size, CidrRecord **cidr_records);

#endif //MERGE_IP_MERGE_H
