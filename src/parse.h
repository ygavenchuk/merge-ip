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


#ifndef MERGE_IP_PARSE_H
#define MERGE_IP_PARSE_H

// Struct to store CIDR blocks and its count
typedef struct {
    const char **cidrs;
    size_t length;
} ParsedData;

void free_parsed_data(ParsedData *data);
ParsedData read_from_stdin();
ParsedData read_from_file(const char *filename);

#endif //MERGE_IP_PARSE_H
