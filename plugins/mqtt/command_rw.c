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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "command_rw.h"

void command_read_once_request(neu_plugin_t *plugin, neu_parse_mqtt_t *mqtt,
                               neu_parse_read_req_t *req)
{
    log_info("READ uuid:%s, node id:%u", mqtt->uuid, req->node_id);

    vector_t nodes = neu_system_get_nodes(plugin, NEU_NODE_TYPE_DRIVER);
    int      rc    = common_node_id_exist(&nodes, req->node_id);
    vector_uninit(&nodes);
    if (0 != rc) {
        return;
    }

    vector_t configs = neu_system_get_group_configs(plugin, req->node_id);
    neu_taggrp_config_t *config;
    neu_taggrp_config_t *c;
    config          = *(neu_taggrp_config_t **) vector_get(&configs, 0);
    c               = (neu_taggrp_config_t *) neu_taggrp_cfg_ref(config);
    uint32_t req_id = neu_plugin_send_read_cmd(plugin, req->node_id, c);

    GROUP_CONFIGS_UINIT(configs);
    UNUSED(req_id);
}

static int wrap_read_response_json_object(neu_fixed_array_t *   array,
                                          neu_parse_read_res_t *json)
{
    neu_int_val_t * int_val;
    neu_data_val_t *val;
    int             length = array->length;
    json->n_tag            = length - 1;

    if (0 == json->n_tag) {
        return -1;
    }

    json->tags = (neu_parse_read_res_tag_t *) calloc(
        length - 1, sizeof(neu_parse_read_res_tag_t));

    for (int i = 1; i < length; i++) {
        int_val          = neu_fixed_array_get(array, i);
        val              = int_val->val;
        neu_dtype_e type = neu_dvalue_get_type(val);

        json->tags[i - 1].name   = strdup((char *) "");
        json->tags[i - 1].tag_id = int_val->key;

        switch (type) {
        case NEU_DTYPE_BYTE: {
            uint8_t value;
            neu_dvalue_get_uint8(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_INT8: {
            int8_t value;
            neu_dvalue_get_int8(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_INT16: {
            int16_t value;
            neu_dvalue_get_int16(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_INT32: {
            int32_t value;
            neu_dvalue_get_int32(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_INT64: {
            int64_t value;
            neu_dvalue_get_int64(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_UINT8: {
            uint8_t value;
            neu_dvalue_get_uint8(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_UINT16: {
            uint16_t value;
            neu_dvalue_get_uint16(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_UINT32: {
            uint32_t value;
            neu_dvalue_get_uint32(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_UINT64: {
            uint64_t value;
            neu_dvalue_get_uint64(val, &value);
            json->tags[i - 1].t             = NEU_JSON_INT;
            json->tags[i - 1].value.val_int = value;
            break;
        }
        case NEU_DTYPE_FLOAT: {
            float value;
            neu_dvalue_get_float(val, &value);
            json->tags[i - 1].t               = NEU_JSON_FLOAT;
            json->tags[i - 1].value.val_float = value;
            break;
        }
        case NEU_DTYPE_DOUBLE: {
            double value;
            neu_dvalue_get_double(val, &value);
            json->tags[i - 1].t                = NEU_JSON_DOUBLE;
            json->tags[i - 1].value.val_double = value;
            break;
        }
        case NEU_DTYPE_CSTR: {
            char *value;
            neu_dvalue_get_ref_cstr(val, &value);
            json->tags[i - 1].t             = NEU_JSON_STR;
            json->tags[i - 1].value.val_str = value;
            break;
        }
        default:
            break;
        }
    }

    return 0;
}

static void clean_read_response_json_object(neu_parse_read_res_t *json)
{
    if (NULL == json) {
        return;
    }

    if (NULL == json->tags) {
        return;
    }

    int length = json->n_tag;
    for (int i = 0; i < length; i++) {
        if (NULL != json->tags[i].name) {
            free(json->tags[i].name);
        }
    }

    free(json->tags);
}

char *command_read_once_response(neu_plugin_t *plugin, const char *uuid,
                                 neu_data_val_t *resp_val)
{
    UNUSED(plugin);
    UNUSED(uuid);
    UNUSED(resp_val);

    neu_fixed_array_t *array;
    int                rc = neu_dvalue_get_ref_array(resp_val, &array);
    if (0 != rc) {
        log_info("Get array ref error");
        return NULL;
    }

    if (0 >= array->length) {
        return NULL;
    }

    neu_int_val_t * int_val;
    neu_data_val_t *val_err;
    int32_t         error;

    int_val = neu_fixed_array_get(array, 0);
    val_err = int_val->val;
    neu_dvalue_get_errorcode(val_err, &error);

    neu_parse_read_res_t json;
    memset(&json, 0, sizeof(neu_parse_read_res_t));
    rc = wrap_read_response_json_object(array, &json);
    if (0 != rc) {
        return NULL;
    }

    char *json_str = NULL;
    rc = neu_json_encode_by_fn(&json, neu_parse_encode_read, &json_str);
    clean_read_response_json_object(&json);
    if (0 != rc) {
        log_info("Json string parse error:%d", rc);
        return NULL;
    }

    return json_str;
}

char *command_read_cycle_response(neu_plugin_t *  plugin,
                                  neu_data_val_t *resp_val)
{
    // TODO: Generate UUID
    return command_read_once_response(plugin, "", resp_val);
}

static int write_command(neu_plugin_t *plugin, uint32_t dest_node_id,
                         neu_data_val_t *write_val)
{
    vector_t nodes = neu_system_get_nodes(plugin, NEU_NODE_TYPE_DRIVER);
    int      rc    = common_node_id_exist(&nodes, dest_node_id);
    vector_uninit(&nodes);

    if (0 != rc) {
        return -1;
    }

    vector_t configs = neu_system_get_group_configs(plugin, dest_node_id);
    neu_taggrp_config_t *config;
    neu_taggrp_config_t *c;
    config = *(neu_taggrp_config_t **) vector_get(&configs, 0);
    c      = (neu_taggrp_config_t *) neu_taggrp_cfg_ref(config);

    uint32_t req_id =
        neu_plugin_send_write_cmd(plugin, dest_node_id, c, write_val);

    GROUP_CONFIGS_UINIT(configs);
    return req_id;
}

static void set_int_type_value(neu_data_val_t *val, const int64_t data)
{
    if (INT8_MIN <= data && INT8_MAX >= data) {
        val = neu_dvalue_new(NEU_DTYPE_INT8);
        neu_dvalue_set_int8(val, (int8_t) data);
        return;
    }
    if (INT16_MIN <= data && INT16_MAX >= data) {
        val = neu_dvalue_new(NEU_DTYPE_INT16);
        neu_dvalue_set_int16(val, (int16_t) data);
        return;
    }
    if (INT32_MIN <= data && INT32_MAX >= data) {
        val = neu_dvalue_new(NEU_DTYPE_INT32);
        neu_dvalue_set_int32(val, (int32_t) data);
        return;
    }
    if (INT64_MIN <= data && INT64_MAX >= data) {
        val = neu_dvalue_new(NEU_DTYPE_INT64);
        neu_dvalue_set_int64(val, (int64_t) data);
        return;
    }
}

void command_write_request(neu_plugin_t *plugin, neu_parse_mqtt_t *mqtt,
                           neu_parse_write_req_t *write_req)
{
    log_info("WRITE uuid:%s", mqtt->uuid);

    neu_data_val_t *   write_val;
    neu_fixed_array_t *array;
    write_val = neu_dvalue_unit_new();
    if (write_val == NULL) {
        log_error("Failed to allocate data value for write data");
        return;
    }

    array = neu_fixed_array_new(write_req->n_tag, sizeof(neu_int_val_t));
    if (array == NULL) {
        log_error("Failed to allocate array for write data");
        neu_dvalue_free(write_val);
        return;
    }

    neu_int_val_t   int_val;
    neu_data_val_t *val;
    for (int i = 0; i < write_req->n_tag; i++) {
        enum neu_json_type   type  = write_req->tags[i].t;
        union neu_json_value value = write_req->tags[i].value;
        neu_datatag_id_t     id    = write_req->tags[i].tag_id;

        switch (type) {
        case NEU_JSON_INT: {
            set_int_type_value(val, value.val_int);
            break;
        }
        case NEU_JSON_BIT: {
            val = neu_dvalue_new(NEU_DTYPE_BIT);
            neu_dvalue_set_bit(val, value.val_bit);
            break;
        }
        case NEU_JSON_STR: {
            val = neu_dvalue_new(NEU_DTYPE_CSTR);
            neu_dvalue_set_cstr(val, strdup(value.val_str));
            break;
        }
        case NEU_JSON_DOUBLE: {
            val = neu_dvalue_new(NEU_DTYPE_DOUBLE);
            neu_dvalue_set_double(val, value.val_double);
            break;
        }
        case NEU_JSON_FLOAT: {
            val = neu_dvalue_new(NEU_DTYPE_FLOAT);
            neu_dvalue_set_float(val, value.val_float);
            break;
        }
        case NEU_JSON_BOOL: {
            val = neu_dvalue_new(NEU_DTYPE_BOOL);
            neu_dvalue_set_bool(val, value.val_bool);
            break;
        }
        default:
            break;
        }

        neu_int_val_init(&int_val, id, val);
        neu_fixed_array_set(array, i, (void *) &int_val);
    }

    neu_dvalue_init_move_array(write_val, NEU_DTYPE_INT_VAL, array);
    write_command(plugin, write_req->node_id, write_val);
    neu_dvalue_free(write_val);
}

static int wrap_write_response_json_object(neu_fixed_array_t *   array,
                                           neu_parse_read_res_t *json)
{
    neu_int_val_t * int_val;
    neu_data_val_t *val;
    int             length = array->length;
    json->n_tag            = length - 1;

    json->tags = (neu_parse_read_res_tag_t *) calloc(
        length - 1, sizeof(neu_parse_read_res_tag_t));

    for (int i = 1; i < length; i++) {
        int_val = neu_fixed_array_get(array, i);
        val     = int_val->val;

        json->tags[i - 1].name   = strdup((char *) "");
        json->tags[i - 1].tag_id = int_val->key;

        int32_t value;
        neu_dvalue_get_int32(val, &value);
        json->tags[i - 1].t             = NEU_JSON_INT;
        json->tags[i - 1].value.val_int = value;
    }

    return 0;
}

static void clean_write_response_json_object(neu_parse_read_res_t *json)
{
    if (NULL == json) {
        return;
    }

    if (NULL == json->tags) {
        return;
    }

    int length = json->n_tag;
    for (int i = 0; i < length; i++) {
        if (NULL != json->tags[i].name) {
            free(json->tags[i].name);
        }
    }

    free(json->tags);
}

char *command_write_response(neu_plugin_t *plugin, const char *uuid,
                             neu_data_val_t *resp_val)
{
    UNUSED(plugin);
    UNUSED(uuid);

    neu_fixed_array_t *array;
    neu_dvalue_get_ref_array(resp_val, &array);

    neu_int_val_t * int_val;
    neu_data_val_t *val_err;
    int32_t         error;

    int_val = neu_fixed_array_get(array, 0);
    val_err = int_val->val;
    neu_dvalue_get_errorcode(val_err, &error);

    neu_parse_read_res_t json;
    wrap_write_response_json_object(array, &json);

    char *json_str = NULL;
    neu_json_encode_by_fn(&json, neu_parse_encode_read, &json_str);
    clean_write_response_json_object(&json);

    return json_str;
}