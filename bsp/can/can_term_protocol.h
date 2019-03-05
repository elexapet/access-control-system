#ifndef CAN_TERM_PROTOCOL_H_
#define CAN_TERM_PROTOCOL_H_

// Node identification
#define CAN_ID_MASTER_0 0x1
#define CAN_ID_MASTER_1 0x2
#define CAN_ID_MASTER_2 0x3
#define CAN_ID_FIRST_SLAVE 0x4
#define CAN_ID_LAST_SLAVE 0xFFFF

// Function codes
#define CAN_MASTER_COMM       0x00000
#define CAN_AUTH_REQ          0x10000
#define CAN_AUTH_RESP         0x20000
#define CAN_REMOTE_LOCK       0x30000
#define CAN_ANNOUNCE_USER_ID  0x40000

#pragma pack(push,1)

typedef struct
{
  uint32_t user_id;
  uint8_t panel_id : 2;
  uint8_t : 6;
} can_auth_req_t;

typedef struct
{
  uint32_t user_id;
  uint8_t status : 2;
  uint8_t p0 : 1;
  uint8_t p1 : 1;
  uint8_t p2 : 1;
  uint8_t p3 : 1;
  uint8_t  : 2;
} can_auth_resp_t;

typedef struct
{
  uint32_t user_id;
  uint8_t p0 : 1;
  uint8_t p1 : 1;
  uint8_t p2 : 1;
  uint8_t p3 : 1;
  uint8_t : 4;
} can_announce_user_id_t;

typedef struct
{
  uint32_t node_id;
  uint8_t panel_id : 2;
  uint8_t : 6;
} can_remote_lock_t;

#pragma pack(pop)

#endif /* CAN_TERM_PROTOCOL_H_ */
