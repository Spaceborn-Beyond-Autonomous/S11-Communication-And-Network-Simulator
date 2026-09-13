/**
 * @file    s11_link.h
 * @author  Sara Saad Mahmoud
 * @date    2026-09-10
 * @brief   Defines the network link state used by the S11 simulator.
 *
 * @details
 * This header provides the common representation of a communication
 * link between two nodes in the S11 Communication & Network Simulator.
 *
 * A link represents the connection between a source node and a
 * destination node and maintains its current runtime state.
 *
 * The link state can represent conditions such as:
 * - Link is available.
 * - Link is disabled.
 * - Link is affected by jamming.
 * - Link is affected by a network partition.
 */

 #ifndef S11_LINK_H
 #define S11_LINK_H
 
 /************************************* Include Part ************************************* */
 
 #include "s11_common.h"
 
 /************************************* Macros Part ************************************* */
 
 /************************************* User Data Types ************************************* */
 
 /**
  * @enum s11_link_state_t
  * @brief Represents the current runtime state of a network link.
 */
 typedef enum
 {
     S11_LINK_UP = 0, /* Link is available. */
 
     S11_LINK_DOWN,   /* Link is disabled. */
 
     S11_LINK_JAMMED, /* Link is affected by jamming. */
 
     S11_LINK_PARTITIONED /* Link is affected by a network partition. */
 
 } s11_link_state_t;
 
 
 /**
 * @struct s11_link_t
 * @brief Represents a communication link between two network nodes.
 */
typedef struct
{
    /**
     * @brief Unique identifier of the link.
    */
    char link_id[S11_LINK_ID_MAX_LEN];

    /**
     * @brief Identifier of the source node.
    */
    char source[S11_NODE_ID_MAX_LEN];

    /**
     * @brief Identifier of the destination node.
    */
    char destination[S11_NODE_ID_MAX_LEN];

    /**
     * @brief Current runtime state of the link.
    */
    s11_link_state_t state;

    /**
     * @brief Physical link type (e.g. "lte", "5g", "mesh", "lora",
     * "satellite"). Used to select the matching s11_link_model_config_t
     * from config/network.yaml.
    */
    char link_type[16];

} s11_link_t;
 /************************************* Function Prototypes Part ************************************* */
 
 
 /************************************* End of File ************************************* */
 
 #endif /* S11_LINK_H */