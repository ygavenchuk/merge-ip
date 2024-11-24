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

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "merge.h"

void test_merge_cidr(void **state) {
    const char *cidr_list[] = {"192.168.0.0/24", "192.168.1.0/24"};
    CidrRecord *cidr_records = NULL;
    size_t count = merge_cidr(cidr_list, 2, &cidr_records);

    assert_int_equal(count, 1);
    assert_string_equal(cidr_records[0], "192.168.0.0/23");

    free(cidr_records);
}
