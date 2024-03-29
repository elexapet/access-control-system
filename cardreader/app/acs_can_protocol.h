/**
 *  @file
 *  @brief ACS CAN protocol
 *
 *  Contains defines and data types for the protocol.
 *  ACS protocol uses messages with extended ID from CAN 2.0B specification.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef ACS_CAN_PROTOCOL_H_
#define ACS_CAN_PROTOCOL_H_

// Used Message object numbers.
#define ACS_MSGOBJ_SEND_DOOR_A 0
#define ACS_MSGOBJ_SEND_DOOR_B 1
#define ACS_MSGOBJ_RECV_DOOR_A 2
#define ACS_MSGOBJ_RECV_DOOR_B 3
#define ACS_MSGOBJ_RECV_BCAST 4

// Message head partition sizes (29b total).
#define ACS_PRIO_BITS   3
#define ACS_FC_BITS     6
#define ACS_ADDR_BITS   10

// Address space.
#define ACS_BROADCAST_ADDR  ((1 << ACS_ADDR_BITS) - 1)
#define ACS_RESERVED_ADDR   0
#define ACS_MSTR_FIRST_ADDR 1
#define ACS_MSTR_LAST_ADDR  3
#define ACS_PNL_FIRST_ADDR  4
#define ACS_PNL_LAST_ADDR   (ACS_BROADCAST_ADDR - 1)

// Message head offsets.
#define ACS_SRC_ADDR_OFFSET 0
#define ACS_DST_ADDR_OFFSET (ACS_SRC_ADDR_OFFSET + ACS_ADDR_BITS)
#define ACS_FC_OFFSET       (ACS_DST_ADDR_OFFSET + ACS_ADDR_BITS)
#define ACS_PRIO_OFFSET     (ACS_FC_OFFSET + ACS_FC_BITS)

// Message head masks.
#define ACS_SRC_ADDR_MASK (((1 << ACS_ADDR_BITS) - 1) << ACS_SRC_ADDR_OFFSET)
#define ACS_DST_ADDR_MASK (((1 << ACS_ADDR_BITS) - 1) << ACS_DST_ADDR_OFFSET)
#define ACS_FC_MASK       (((1 << ACS_FC_BITS) - 1) << ACS_FC_OFFSET)
#define ACS_PRIO_MASK     (((1 << ACS_PRIO_BITS) - 1) << ACS_PRIO_OFFSET)

// Function codes used in ACS protocol.
#define FC_RESERVED            0x0
#define FC_USER_AUTH_REQ       0x1 // S -> M
#define FC_USER_NOT_AUTH_RESP  0x2 // M -> S
#define FC_USER_AUTH_RESP      0x3 // M -> S
#define FC_DOOR_CTRL           0x4 // M -> S
#define FC_DOOR_STATUS         0x5 // S -> M
#define FC_ALIVE               0x6 // M -> S
#define FC_LEARN_USER          0x7 // S -> M
#define FC_LEARN_USER_OK       0x8 // M -> S
#define FC_LEARN_USER_FAIL     0x9 // M -> S

// Priority range.
#define ACS_MAX_PRIO  0
#define ACS_LOW_PRIO  ((1 << ACS_PRIO_BITS) - 1)

// Priorities for each function code (ACS_MAX_PRIO to ACS_LOW_PRIO).
#define PRIO_RESERVED            0x0
#define PRIO_USER_AUTH_REQ       0x2
#define PRIO_USER_AUTH_RESP_FAIL 0x2
#define PRIO_USER_AUTH_RESP_OK   0x2
#define PRIO_DOOR_CTRL           0x3
#define PRIO_DOOR_STATUS         0x4
#define PRIO_ALIVE               0x1
#define PRIO_LEARN_USER          0x2
#define PRIO_LEARN_USER_FAIL     0x2
#define PRIO_LEARN_USER_OK       0x2

// Data for FC_DOOR_CTRL.
#define DATA_DOOR_CTRL_REMOTE_UNLCK 0x01
#define DATA_DOOR_CTRL_CLR_CACHE    0x02
#define DATA_DOOR_CTRL_LEARN_MODE   0x03
#define DATA_DOOR_CTRL_NORMAL_MODE  0x04

// Data for FC_DOOR_STATUS.
#define DATA_DOOR_STATUS_CLOSED   0x01
#define DATA_DOOR_STATUS_OPEN     0x02

// Setting for master communication status.
#define ACS_MASTER_ALIVE_PERIOD_MS  5000
#define ACS_MASTER_ALIVE_TIMEOUT_MS 10000


/*
 * Data types definitions.
 */

#pragma pack(push,1)

// Structure describing the message head (CAN ID).
typedef union
{
  struct
  {
    uint32_t src : ACS_ADDR_BITS;
    uint32_t dst : ACS_ADDR_BITS;
    uint32_t fc : ACS_FC_BITS;
    uint32_t prio : ACS_PRIO_BITS;
    uint32_t flags : 3;
  };
  uint32_t scalar;
} acs_msg_head_t;

// Structure of data sent with FC_USER_AUTH_REQ.
typedef struct
{
  uint32_t user_id;
} acs_msg_data_auth_req_t;

// Structure of data sent with FC_USER_AUTH_RESP and FC_USER_NOT_AUTH_RESP.
typedef struct
{
  uint32_t user_id;
} acs_msg_data_auth_resp_t;

// Structure of data sent with FC_DOOR_CTRL.
typedef struct
{
  uint8_t ctrl_command;
} acs_msg_data_door_ctrl_t;

#pragma pack(pop)

#endif /* ACS_CAN_PROTOCOL_H_ */
