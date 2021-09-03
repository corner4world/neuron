/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2021 EMQ Technologies Co., Ltd All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <stdlib.h>
#include <string.h>

#include "utils/json.h"

#include "neu_json_tag_group.h"

#define GROUP "group"
#define NAMES "names"
#define NAME "name"
#define TYPE "type"
#define TIMESTAMP "timestamp"
#define VALUE "value"
#define TAGS "tags"

int neu_paser_decode_read_tag_group_list_req(
    char *buf, struct neu_paser_read_tag_group_list_req **result)
{
    struct neu_paser_read_tag_group_list_req *req =
        malloc(sizeof(struct neu_paser_read_tag_group_list_req));
    memset(req, 0, sizeof(struct neu_paser_read_tag_group_list_req));

    neu_json_elem_t elem[] = { {
                                   .name = NEU_PARSE_UUID,
                                   .t    = NEU_JSON_STR,
                               },
                               {
                                   .name = "config",
                                   .t    = NEU_JSON_STR,
                               }

    };

    int ret = neu_json_decode(buf, NEU_JSON_ELEM_SIZE(elem), elem);
    if (ret != 0) {
        free(req);
        return -1;
    }
    req->function = NEU_PARSE_OP_READ_TAG_GROUP_LIST;
    req->uuid     = elem[0].v.val_str;
    req->config   = elem[1].v.val_str;

    *result = req;
    return 0;
}

int neu_parse_encode_read_tag_group_list_res(
    struct neu_paser_read_tag_group_list_res *res, char **buf)
{
    void *array = NULL;
    for (int i = 0; i < res->n_group; i++) {
        neu_json_elem_t group[] = { {
            .name      = NAME,
            .t         = NEU_JSON_STR,
            .v.val_str = res->names[i].name,
        } };
        array                   = neu_json_encode_array(array, group, 1);
    }
    neu_json_elem_t elems[] = { {
                                    .name = NEU_PARSE_FUNCTION,
                                    .t    = NEU_JSON_INT,
                                    .v.val_int =
                                        NEU_PARSE_OP_READ_TAG_GROUP_LIST,
                                },
                                {
                                    .name      = NEU_PARSE_UUID,
                                    .t         = NEU_JSON_STR,
                                    .v.val_str = res->uuid,
                                },
                                {
                                    .name      = NEU_PARSE_ERROR,
                                    .t         = NEU_JSON_INT,
                                    .v.val_int = res->error,
                                },
                                {
                                    .name     = "groups",
                                    .t        = NEU_JSON_OBJECT,
                                    .v.object = array,
                                } };
    return neu_json_encode(elems, 4, buf);
}