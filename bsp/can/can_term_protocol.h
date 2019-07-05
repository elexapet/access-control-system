#ifndef CAN_TERM_PROTOCOL_H_
#define CAN_TERM_PROTOCOL_H_


// HEAD partitions sizes
#define ACS_PRIO_BITS   3
#define ACS_FC_BITS     6
#define ACS_ADDR_BITS   10

// Address space
#define ACS_BROADCAST_ADDR  ((1 << ACS_ADDR_BITS) - 1)
#define ACS_MSTR_FIRST_ADDR 0
#define ACS_MSTR_LAST_ADDR  1
#define ACS_PNL_FIRST_ADDR  2
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
#define FC_USER_AUTH_RESP_FAIL 0x2
#define FC_USER_AUTH_RESP_OK   0x3
#define FC_PANEL_CTRL          0x4
#define FC_NEW_USER            0x5
#define FC_DOOR_STATUS         0x6

// priorities
#define PRIO_RESERVED            0x0
#define PRIO_USER_AUTH_REQ       0x2
#define PRIO_USER_AUTH_RESP_FAIL 0x2
#define PRIO_USER_AUTH_RESP_OK   0x2
#define PRIO_PANEL_CTRL          0x4
#define PRIO_NEW_USER            0x4
#define PRIO_DOOR_STATUS         0x3


#define PANEL_CTRL_DATA_DEF   0x00
#define PANEL_CTRL_DATA_UNLCK 0x01
#define PANEL_CTRL_DATA_LCK   0x02
#define PANEL_CTRL_DATA_LEARN 0x03
#define PANEL_CTRL_DATA_CLR_CACHE 0x04

#define DOOR_STATUS_DATA_CLOSED 0x0
#define DOOR_STATUS_DATA_OPEN 0x1

// data types

#pragma pack(push,1)

typedef struct
{
  uint32_t prio : ACS_PRIO_BITS;
  uint32_t fc : ACS_FC_BITS;
  uint32_t dst : ACS_ADDR_BITS;
  uint32_t src : ACS_ADDR_BITS;
} msg_head_t;

typedef struct
{
  uint32_t user_id;
} msg_data_auth_req_t;

typedef struct
{
  uint32_t user_id;
} masg_data_auth_resp_t;

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
