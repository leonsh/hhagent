#include <hhp.h>
#include "emsnmp/emsnmp.h"

struct msg_entry msg_data[] = {

	/* KJD Entry */
	/* on/off1 Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH,
		13, 13, 1, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 1, 0},
		INTEGER
	},
	/* on/off1 status Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH_STATUS,
		13, 0, 1, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 2, 0},
		INTEGER
	},
	/* on/off2 Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH,
		13, 13, 2, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 3, 0},
		INTEGER
	},
	/* on/off2 status Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH_STATUS,
		13, 0, 2, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 4, 0},
		INTEGER
	},
	/* on/off3 Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH,
		13, 13, 3, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 5, 0},
		INTEGER
	},
	/* on/off3 status Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH_STATUS,
		13, 0, 3, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 6, 0},
		INTEGER
	},
	/* on/off4 Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH,
		13, 13, 4, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 7, 0},
		INTEGER
	},
	/* on/off4 status Entry */
	{
		HH_DEV_SWITCH, HH_OID_SWITCH_STATUS,
		13, 0, 4, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 8, 0},
		INTEGER
	},
    /* ION 控制开关 */
	{
		HH_DEV_SWITCH, HH_OID_ION,
		11, 12, 0, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 100, 1, 0},
		INTEGER
	},
    /* ION 开关状态 */
	{
		HH_DEV_SWITCH, HH_OID_ION_STATUS,
		11, 0, 0, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 100, 2, 0},
		INTEGER
	},
    /* ION 用电量 */
	{
		HH_DEV_SWITCH, HH_OID_ION_ELEC,
		11, 12, 0, {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 100, 3, 0},
		INTEGER
	}
};

const int max_entries = (sizeof(msg_data) / sizeof(struct msg_entry));
