/**
 * @file    s11_packet.h
 * @author  Sara Saad Mahmoud
 * @date    2026-09-10
 * @brief   Defines the generic packet structure used by the S11 simulator.
 *
 * @details
 * This header provides the common packet representation used throughout
 * the S11 Communication & Network Simulator.
 *
 * The packet structure is protocol-independent and can represent traffic
 * from different communication protocols such as ROS 2, DDS, and MAVLink.
 *
 * The packet contains:
 * - A unique packet identifier.
 * - Source and destination node identifiers.
 * - Communication protocol information.
 * - Raw payload data and its size.
 * - Simulation timestamp.
 * 
 * The packet management API provides functions for: 
 * - Creating packets. 
 * - Validating packets.
 * - Destroying packets and releasing their resources.
 */

 #ifndef S11_PACKET_H
 #define S11_PACKET_H
 
 /************************************* Include Part ************************************* */
 
 #include <stdint.h>
 #include <stdbool.h>
 #include "s11_protocol.h"
 #include "s11_common.h"
 
 /************************************* Macros Part ************************************* */
 
 
 /************************************* User Data Types Part ************************************* */
 
 /**
  * @struct s11_packet_t
  * @brief  Represents a generic communication packet in S11.
  */
 typedef struct
 {
     /**
      * @brief Unique identifier of the packet.
      */
     uint64_t packet_id;
 
     /**
      * @brief Identifier of the node that generated the packet.
      */
     char source[S11_NODE_ID_MAX_LEN];
 
     /**
      * @brief Identifier of the node that should receive the packet.
      */
     char destination[S11_NODE_ID_MAX_LEN];
 
     /**
      * @brief Communication protocol associated with the packet.
      */
     s11_protocol_t protocol;
 
     /**
      * @brief Pointer to the raw data carried by the packet.
      *
      * @details
      * The payload is represented as raw bytes to keep the packet protocol-independent.
      *
      * @note
      * The packet owns the payload memory. The packet manager is
      * responsible for allocating and releasing the payload.
      */
     uint8_t *payload;
 
     /**
      * @brief Size of the payload in bytes.
      */
     uint32_t payload_size;
 
     /**
      * @brief Simulation time when the packet was created.
      *
      * @note
      * The timestamp is expressed in microseconds and represents
      * simulation time rather than wall-clock time.
      */
     uint64_t timestamp_us;
 
 } s11_packet_t;
 
 /************************************* Function Prototypes Part ************************************* */
 /** 
   * @brief Creates and initializes a new S11 packet. 
   * @details 
   * This function allocates memory for a new packet and creates an 
   * independent copy of the provided payload. 
   * 
   * The caller retains ownership of the input payload. The newly 
   * created packet owns its internal payload copy. 
   * 
   * @param[in] source 
   * Identifier of the node that generated the packet. 
   * @param[in] destination 
   * Identifier of the node that should receive the packet. 
   * @param[in] protocol 
   * Communication protocol associated with the packet. 
   * @param[in] payload 
   * Pointer to the payload data to be copied into the packet. 
   * @param[in] payload_size 
   * Size of the payload in bytes. 
   * @param[in] timestamp_us 
   * Simulation timestamp in microseconds. 
   * 
   * @return 
   * Pointer to the newly created packet on success. 
   * NULL if the packet could not be created. 
 */
 s11_packet_t *s11_packet_create(const char *source, 
                                 const char *destination, 
                                 s11_protocol_t protocol, 
                                 const uint8_t *payload, 
                                 uint32_t payload_size, 
                                 uint64_t timestamp_us 
                             );
 
 /**
  * @brief Destroys an S11 packet and releases its resources.
  *
  * @details
  * This function releases the memory allocated for the packet and
  * its internally owned payload.
  *
  * @param[in] packet
  *      Pointer to the packet to be destroyed.
  */
 void s11_packet_destroy(s11_packet_t *packet);
 
 
 /**
  * @brief Validates an S11 packet.
  *
  * @details
  * This function checks whether the packet contains valid information
  * required for processing within the S11 simulator.
  *
  * @param[in] packet
  *      Pointer to the packet to be validated.
  *
  * @return
  *      true if the packet is valid.
  *      false otherwise.
  */
 bool s11_packet_validate(const s11_packet_t *packet);
 
 
 /************************************* End of File ************************************* */
 
 #endif /* S11_PACKET_H */
 