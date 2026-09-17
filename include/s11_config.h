/**
* @file    s11_config.h
* @author  Sara Saad Mahmoud
* @date    2026-09-13
* @brief   Defines the network configuration structure and public APIs.
*
* @details
* This header provides the configuration representation and public
* interfaces used by the S11 Communication & Network Simulator.
*
* The configuration structure represents the network conditions that
* can be applied during packet processing, including latency, jitter,
* packet loss, and bandwidth.
*/

#ifndef S11_CONFIG_H
#define S11_CONFIG_H

/************************************* Include Part ************************************* */

#include <stdbool.h>
#include "s11_model.h"

/************************************* Macros Part ************************************* */

/************************************* User Data Types Part ************************************* */

/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Initializes a network configuration with default values.
 *
 * @param[out] config
 * Pointer to the network configuration to be initialized.
 *
 * @return
 * true if the configuration was initialized successfully.
 * false if the configuration pointer is NULL.
*/
bool s11_config_init( s11_link_model_config_t*config );


/**
 * @brief Validates the network configuration parameters.
 *
 * @param[in] config
 * Pointer to the network configuration to be validated.
 *
 * @return
 * true if all configuration parameters are valid.
 * false if the configuration is NULL or contains invalid values.
*/
bool s11_config_validate( const s11_link_model_config_t *config );

/**
 * @brief Loads network configuration values from a configuration file.
 *
 * @param[out] config
 * Pointer to the network configuration to be populated.
 *
 * @param[in] config_file
 * Path to the configuration file.
 *
 * @return
 * true if the configuration was loaded successfully.
 * false if the configuration could not be loaded or is invalid.
*/
bool s11_config_load( s11_link_model_config_t *config, const char *config_file, const char *link_type);

/************************************* End of File ************************************* */

#endif /* S11_CONFIG_H */
