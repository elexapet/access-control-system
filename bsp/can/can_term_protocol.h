// ACS protocol definitions

#ifndef CAN_TERM_PROTOCOL_H_
#define CAN_TERM_PROTOCOL_H_

// Used Message object numbers
#define ACS_MSGOBJ_SEND_DOOR_A 0
#define ACS_MSGOBJ_SEND_DOOR_B 1
#define ACS_MSGOBJ_RECV_DOOR_A 2
#define ACS_MSGOBJ_RECV_DOOR_B 3
#define ACS_MSGOBJ_RECV_BCAST 4

// HEAD partitions sizes
#define ACS_PRIO_BITS   3
#define ACS_FC_BITS     6
#define ACS_ADDR_BITS   10

// Address space
#define ACS_BROADCAST_ADDR  ((1 << ACS_ADDR_BITS) - 1)
#define ACS_RESERVED_ADDR   0
#define ACS_MSTR_FIRST_ADDR 1
#define ACS_MSTR_LAST_ADDR  3
#define ACS_PNL_FIRST_ADDR  4
#define ACS_PNL_LAST_ADDR   (ACS_BROADCAST_ADDR - 1)

// message head offsets
#define ACS_SRC_ADDR_OFFSET 0
#define ACS_DST_ADDR_OFFSET (ACS_SRC_ADDR_OFFSET + ACS_ADDR_BITS)
#define ACS_FC_OFFSET       (ACS_DST_ADDR_OFFSET + ACS_ADDR_BITS)
#define ACS_PRIO_OFFSET     (ACS_FC_OFFSET + ACS_FC_BITS)

// message head masks
#define ACS_SRC_ADDR_MASK (((1 << ACS_ADDR_BITS) - 1) << ACS_SRC_ADDR_OFFSET)
#define ACS_DST_ADDR_MASK (((1 << ACS_ADDR_BITS) - 1) << ACS_DST_ADDR_OFFSET)
#define ACS_FC_MASK       (((1 << ACS_FC_BITS) - 1) << ACS_FC_OFFSET)
#define ACS_PRIO_MASK     (((1 << ACS_PRIO_BITS) - 1) << ACS_PRIO_OFFSET)

#define ACC_MAX_PRIO  0
#define ACS_LOW_PRIO  ((1 << ACS_PRIO_BITS) - 1)

// ACS protocol function codes
#define FC_RESERVED            0x0
#define FC_USER_AUTH_REQ       0x1
#define FC_USER_NOT_AUTH_RESP  0x2
#define FC_USER_AUTH_RESP      0x3
#define FC_PANEL_CTRL          0x4
#define FC_NEW_USER            0x5
#define FC_DOOR_STATUS         0x6
#define FC_ALIVE               0x7

// priorities for each function code (0-7)
#define PRIO_RESERVED            0x0
#define PRIO_USER_AUTH_REQ       0x2
#define PRIO_USER_AUTH_RESP_FAIL 0x2
#define PRIO_USER_AUTH_RESP_OK   0x2
#define PRIO_PANEL_CTRL          0x3
#define PRIO_NEW_USER            0x3
#define PRIO_DOOR_STATUS         0x4
#define PRIO_ALIVE               0x1


#define PANEL_CTRL_DATA_DEF   0x00
#define PANEL_CTRL_DATA_UNLCK 0x01
#define PANEL_CTRL_DATA_LOCK  0x02
#define PANEL_CTRL_DATA_LEARN 0x03
#define PANEL_CTRL_DATA_CLR_CACHE 0x04

#define DOOR_STATUS_DATA_CLOSED 0x0
#define DOOR_STATUS_DATA_OPEN 0x1

#define MASTER_ALIVE_PERIOD  5000   // ms
#define MASTER_ALIVE_TIMEOUT 10000  // ms

// data types

#pragma pack(push,1)

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
} msg_head_t;

typedef struct
{
  uint32_t user_id;
} msg_data_auth_req_t;

typedef struct
{
  uint32_t user_id;
} msg_data_auth_resp_t;

typedef struct
{
  uint32_t user_id;
} msg_data_new_user_t;

typedef struct
{
  uint8_t ctrl_command;
} msg_data_panel_ctrl_t;

#pragma pack(pop)

#endif /* CAN_TERM_PROTOCOL_H_ */
