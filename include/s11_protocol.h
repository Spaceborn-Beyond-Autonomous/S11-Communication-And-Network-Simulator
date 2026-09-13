/**
 * @file    s11_protocol.h
 * @author  Sara Saad Mahmoud
 * @date    2026-09-10
 * @brief   Defines the communication protocols supported by the S11 simulator.
 *
 * @details
 * This header provides the common protocol representation used throughout
 * the S11 Communication & Network Simulator.
 *
 * The protocol representation is used to identify the communication
 * protocol associated with each packet while keeping the packet structure
 * protocol-independent.
 *
 * The supported protocols include:
 * - ROS 2.
 * - DDS.
 * - MAVLink.
*/

#ifndef S11_PROTOCOL_H
#define S11_PROTOCOL_H

/************************************* Include Part ************************************* */


/************************************* Macros Part ************************************* */


/************************************* User Data Types ************************************* */

/**
 * @enum s11_protocol_t
 * @brief Represents the communication protocol associated with a packet.
*/
typedef enum
{
    S11_PROTOCOL_UNKNOWN = 0,

    S11_PROTOCOL_ROS2,

    S11_PROTOCOL_DDS,

    S11_PROTOCOL_MAVLINK

} s11_protocol_t;


/************************************* Function Prototypes Part ************************************* */


/************************************* End of File ************************************* */

#endif /* S11_PROTOCOL_H */