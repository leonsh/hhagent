#ifndef _HHP_CORE_H_
#define _HHP_CORE_H_
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Protocol command define */
#define HH_PROT_COMM             0x00
#define HH_PROT_JOIN             0xc0

#define MAX_DATA_LENGTH	         54
#define HH_PROT_DATACRC_LEN      60

COMPILER_PACK_SET(1)
struct hh_prot
{
	uint8_t		ctl;
	uint16_t	counter;
	uint8_t		subPkt_count;
	uint8_t		subPkt_index;
	uint8_t		dataLen;
	uint8_t		data[MAX_DATA_LENGTH];
	uint16_t	crc;
};
COMPILER_PACK_RESET()

COMPILER_PACK_SET(1)
struct hh_prot_dev_info
{
	uint32_t        type;
	uint16_t        saddr;
	uint8_t         mac[8];
	uint32_t        vendor;
	uint16_t        hw_ver;
	uint16_t        sw_ver;
};
COMPILER_PACK_RESET()

#ifdef __cplusplus
}
#endif
#endif // _HHP_CORE_H_
