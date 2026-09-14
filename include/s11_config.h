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

/************************************* Macros Part ************************************* */

/************************************* User Data Types Part ************************************* */

/**
 * @struct s11_network_config_t
 * @brief Represents the configurable network conditions of the S11 simulator.
*/
typedef struct
{
    /**
      * @brief Network latency in milliseconds.
    */
    double latency_ms;

    /**
     * @brief Network jitter in milliseconds.
     */
    double jitter_ms;

    /**
     * @brief Packet loss percentage.
     */
    double packet_loss_percent;

    /**
     * @brief Network bandwidth in megabits per second.
     */
    double bandwidth_mbps;

} s11_network_config_t;

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
bool s11_config_init( s11_network_config_t *config );


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
bool s11_config_validate( const s11_network_config_t *config );

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
bool s11_config_load( s11_network_config_t *config, const char *config_file );

/************************************* End of File ************************************* */

#endif /* S11_CONFIG_H */
