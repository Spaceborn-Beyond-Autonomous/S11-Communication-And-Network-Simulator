/**
 * @file    s11_config.c
 * @author  Sara Saad Mahmoud
 * @date    2026-09-13
 * @brief   Implements network configuration loading and validation.
 *
 * @details
 * This source file provides the implementation of the configuration
 * management APIs used by the S11 Communication & Network Simulator.
 *
 * The configuration is loaded from a YAML file containing link-specific
 * communication parameters such as latency, jitter, packet loss,
 * bandwidth, and queue capacity.
 */

#include "s11_config.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <yaml.h>


/************************************* Private Macros Part ************************************* */

#define S11_YAML_MAX_DEPTH       16U
#define S11_YAML_KEY_MAX_LEN     64U


/************************************* Private Function Prototypes ************************************* */

static bool yaml_scalar_to_double(
    const yaml_event_t *event,
    double *value
);

static bool yaml_scalar_to_uint32(
    const yaml_event_t *event,
    uint32_t *value
);


/************************************* Function Definitions ************************************* */

bool s11_config_init(s11_link_model_config_t *config)
{
    if(config == NULL)
    {
        return (false);
    }

    config->link_type[0] = '\0';

    config->packet_loss_rate = 0.0;

    config->queue_capacity = 0U;

    config->jitter_min_ms = 0.0;
    config->jitter_max_ms = 0.0;

    config->base_latency_ms = 0.0;

    config->bandwidth_bps = 0.0;

    return (true);
}


bool s11_config_validate(const s11_link_model_config_t *config)
{
    if(config == NULL)
    {
        return (false);
    }

    /*
     * A link type is required to identify the configuration.
     */
    if(config->link_type[0] == '\0')
    {
        return (false);
    }

    /*
     * Packet loss is represented as a probability in [0.0, 1.0].
     *
     * Example:
     * 0.01  = 1%
     * 0.005 = 0.5%
     */
    if((config->packet_loss_rate < 0.0) ||
       (config->packet_loss_rate > 1.0))
    {
        return (false);
    }

    /*
     * A valid link must have a positive bandwidth.
     */
    if(config->bandwidth_bps <= 0.0)
    {
        return (false);
    }

    /*
     * Jitter values cannot be negative, and maximum jitter
     * must not be smaller than minimum jitter.
     */
    if((config->jitter_min_ms < 0.0) ||
       (config->jitter_max_ms < 0.0) ||
       (config->jitter_max_ms < config->jitter_min_ms))
    {
        return (false);
    }

    /*
     * A valid link must have a non-zero queue capacity.
     */
    if(config->queue_capacity == 0U)
    {
        return (false);
    }

    /*
     * Latency cannot be negative.
     */
    if(config->base_latency_ms < 0.0)
    {
        return (false);
    }

    return (true);
}


bool s11_config_load( s11_link_model_config_t *config,
                      const char *config_file,
                      const char *link_type )
{
    FILE *file = NULL;
    yaml_parser_t parser;
    yaml_event_t event;

    bool parser_initialized = false;
    bool parsing = true;
    bool success = false;

    unsigned int depth = 0U;

    bool expecting_key[S11_YAML_MAX_DEPTH] = { true };

    char keys[S11_YAML_MAX_DEPTH][S11_YAML_KEY_MAX_LEN];

    bool target_link_found = false;

    if((config == NULL) || (config_file == NULL) ||
       (link_type == NULL))
    {
        return (false);
    }

    if(link_type[0] == '\0')
    {
        return (false);
    }

    if(strlen(link_type) >= sizeof(config->link_type))
    {
        return (false);
    }

    /*
     * Start with a clean configuration.
     */
    if(!s11_config_init(config))
    {
        return (false);
    }

    file = fopen(config_file, "r");
    if(file == NULL)
    {
        return (false);
    }

    /*
     * Initialize the YAML parser.
     */
    if(!yaml_parser_initialize(&parser))
    {
        fclose(file);

        return (false);
    }

    parser_initialized = true;

    /*
     * Connect the parser to the configuration file.
     */
    yaml_parser_set_input_file(&parser, file);

    while(parsing)
    {
        if(!yaml_parser_parse(&parser, &event))
        {
            goto cleanup;
        }

        switch(event.type)
        {
            /*
             * End of YAML stream.
             */
            case YAML_STREAM_END_EVENT:
            {
                parsing = false;

                break;
            }

            /*
             * Enter a new mapping.
             */
            case YAML_MAPPING_START_EVENT:
            {
                if(depth >= S11_YAML_MAX_DEPTH)
                {
                    yaml_event_delete(&event);

                    goto cleanup;
                }

                expecting_key[depth] = true;
                keys[depth][0] = '\0';

                depth++;

                break;
            }

            /*
             * Leave the current mapping.
             */
            case YAML_MAPPING_END_EVENT:
            {
                if(depth == 0U)
                {
                    yaml_event_delete(&event);

                    goto cleanup;
                }

                depth--;

                /*
                 * The parent mapping is ready for its next key.
                 */
                if(depth > 0U)
                {
                    expecting_key[depth - 1U] = true;
                }

                /*
                 * Leaving the selected link itself.
                 *
                 * YAML structure:
                 *
                 * links:
                 *   lte:
                 *     ...
                 *
                 * The selected link mapping starts at depth 3
                 * and ends when depth returns to 2.
                 */
                if((depth == 2U) && target_link_found)
                {
                    parsing = false;
                }

                break;
            }

            /*
             * Process YAML scalar values.
             */
            case YAML_SCALAR_EVENT:
            {
                const char *value =
                    (const char *)event.data.scalar.value;

                size_t value_length =
                    event.data.scalar.length;

                if(depth == 0U)
                {
                    yaml_event_delete(&event);

                    goto cleanup;
                }

                /*
                 * The first scalar in a mapping is a key.
                 */
                if(expecting_key[depth - 1U])
                {
                    if(value_length >= S11_YAML_KEY_MAX_LEN)
                    {
                        yaml_event_delete(&event);

                        goto cleanup;
                    }

                    strcpy(
                        keys[depth - 1U],
                        value
                    );

                    /*
                     * Check whether the current key is
                     * the requested link type.
                     *
                     * Example:
                     *
                     * links:
                     *   lte:
                     */
                    if((depth == 2U) &&
                       (strcmp(value, link_type) == 0))
                    {
                        target_link_found = true;

                        strcpy(
                            config->link_type,
                            link_type
                        );
                    }

                    expecting_key[depth - 1U] = false;
                }
                else if((depth == 4U) && target_link_found)
                {
                    double double_value;
                    uint32_t uint32_value;

                    /*
                     * Process parameters inside the
                     * selected link configuration.
                     *
                     * Example:
                     *
                     * lte:
                     *   latency:
                     *     base_ms: 50
                     */
                    if(strcmp(keys[2], "latency") == 0)
                    {
                        if(strcmp(keys[3], "base_ms") == 0)
                        {
                            if(!yaml_scalar_to_double(
                                   &event,
                                   &double_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->base_latency_ms =
                                double_value;
                        }
                    }
                    else if(strcmp(keys[2], "jitter") == 0)
                    {
                        if(strcmp(keys[3], "min_ms") == 0)
                        {
                            if(!yaml_scalar_to_double(
                                   &event,
                                   &double_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->jitter_min_ms =
                                double_value;
                        }
                        else if(strcmp(keys[3], "max_ms") == 0)
                        {
                            if(!yaml_scalar_to_double(
                                   &event,
                                   &double_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->jitter_max_ms =
                                double_value;
                        }
                    }
                    else if(strcmp(keys[2], "packet_loss") == 0)
                    {
                        if(strcmp(keys[3], "rate") == 0)
                        {
                            if(!yaml_scalar_to_double(
                                   &event,
                                   &double_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->packet_loss_rate =
                                double_value;
                        }
                    }
                    else if(strcmp(keys[2], "bandwidth") == 0)
                    {
                        if(strcmp(keys[3], "max_bps") == 0)
                        {
                            if(!yaml_scalar_to_double(
                                   &event,
                                   &double_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->bandwidth_bps =
                                double_value;
                        }
                        else if(strcmp(keys[3],
                                       "queue_capacity") == 0)
                        {
                            if(!yaml_scalar_to_uint32(
                                   &event,
                                   &uint32_value))
                            {
                                yaml_event_delete(&event);

                                goto cleanup;
                            }

                            config->queue_capacity =
                                uint32_value;
                        }
                    }

                    /*
                     * The value has been processed.
                     * The next scalar in this mapping
                     * must be a key.
                     */
                    expecting_key[depth - 1U] = true;
                }
                else
                {
                    /*
                     * The scalar does not belong to the
                     * selected link configuration.
                     */
                    expecting_key[depth - 1U] = true;
                }

                break;
            }

            default:
            {
                break;
            }
        }

        yaml_event_delete(&event);
    }

    /*
     * The requested link must exist.
     */
    if(!target_link_found)
    {
        goto cleanup;
    }

    /*
     * Validate the loaded configuration.
     */
    if(!s11_config_validate(config))
    {
        goto cleanup;
    }

    success = true;

cleanup:

    if(parser_initialized)
    {
        yaml_parser_delete(&parser);
    }

    fclose(file);

    return (success);
}

static bool yaml_scalar_to_double(
    const yaml_event_t *event,
    double *value)
{
    char *endptr;
    double parsed_value;

    if((event == NULL) || (value == NULL) ||
       (event->type != YAML_SCALAR_EVENT))
    {
        return (false);
    }

    parsed_value = strtod(
        (const char *)event->data.scalar.value,
        &endptr
    );

    if(endptr == (char *)event->data.scalar.value)
    {
        return (false);
    }

    if(*endptr != '\0')
    {
        return (false);
    }

    *value = parsed_value;

    return (true);
}


static bool yaml_scalar_to_uint32(
    const yaml_event_t *event,
    uint32_t *value)
{
    char *endptr;
    unsigned long parsed_value;

    if((event == NULL) || (value == NULL) ||
       (event->type != YAML_SCALAR_EVENT))
    {
        return (false);
    }

    parsed_value = strtoul(
        (const char *)event->data.scalar.value,
        &endptr,
        10
    );

    if(endptr == (char *)event->data.scalar.value)
    {
        return (false);
    }

    if(*endptr != '\0')
    {
        return (false);
    }

    if(parsed_value > UINT32_MAX)
    {
        return (false);
    }

    *value = (uint32_t)parsed_value;

    return (true);
}