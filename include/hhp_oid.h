#ifndef _HHP_MSG_ENTRY_H_
#define _HHP_MSG_ENTRY_H_
enum msg_item_type {
	MSG_ENTRY_ONOFF,
};

#define MAX_OID		14
#define MAX_STRING	40

struct msg_entry {
	enum hh_dev_type devType;
	enum hh_oid_type oidType;
	uint8_t oidlen;
	uint8_t index; /*!< Entry Index */
	uint8_t oid[MAX_OID];
	uint8_t dataType;
	uint16_t *dataIndex /*!< Data index in the receive message buffer */
};

#endif /* _HHP_MSG_ENTRY_H_ */