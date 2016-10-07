#include <hhp.h>
#include "hhp_core.h"
#include "crc16.h"
#include <string.h>
#include "emsnmp/emsnmp.h"

static uint16_t msg_counter = 1;
static int32_t hh_send_data(struct hh_prot *p);
static void (* hw_usart_send)(uint8_t);
static void (* sw_dev_join_cb)(struct hh_dev_info *, enum hh_join_stage);
static int32_t hh_wait_for_ctrl_msg(uint8_t counter);
static enum hh_msg_type _hh_process_incoming_pkg();
static uint16_t hh_get_counter();
/**
 * System Operation Indicator, When Set all Data communication with Sub Device
 * Will be discard. The Flag will be set when send HH_CONN_JOINING message to
 * Zigbee
 */
//static bool _hh_sys_operation;

/**
 * Sending/receiving Message in operation.
 */
//static bool _hh_msg_busy;

/**
 * Receive a new package in RAM.
 */
static bool _hh_msg_ready;

/**
 * Receive Package from USART
 */
static struct hh_prot recv_pkg;

/**
 * Zigbee Currently status,
 */
static enum hh_conn_status status = HH_CONN_ONLINE;

static uint16_t hh_get_counter() {
	return msg_counter++;
}

int32_t hh_switch_status(enum hh_conn_status status)
{
	struct hh_prot p;
    uint8_t i;

	memset(&p, 0, sizeof(struct hh_prot));
	p.ctl = HH_CONN_CTRL;
	p.counter = hh_get_counter();
	p.subPkt_count = 1;
	p.subPkt_index = 1;
	p.dataLen = 2;
	p.data[0] = 0x23;
	p.data[1] = (status == HH_CONN_ONLINE) ? 0x00 : 0x01;

    HHP_DEBUG("[Control]Switch Zigbee Status to %s", (status == HH_CONN_ONLINE)?"Communication Mode":"Control Mode");
	hh_send_data(&p);

	return HHP_STATUS_OK;
}

static int32_t hh_send_data(struct hh_prot *p)
{
	uint8_t i = 0;
	if (!hw_usart_send) {
		return HHP_NOT_INITIALIZED;
	}
	p->crc = crc16_data2(((uint8_t *)p), HH_PROT_DATACRC_LEN, 0);

	hw_usart_send(0xF0);
	hw_usart_send(0xA5);
	for (i = 0; i < sizeof(struct hh_prot); i++) {
		hw_usart_send(((uint8_t *)p)[i]);
	}

    HHP_DEBUG_HEXSTRING(i, ((uint8_t *)p), sizeof(struct hh_prot));

	return 64;
}

void hh_usart_reg_write_func(void (* usart_send)(uint8_t))
{
	hw_usart_send = usart_send;
}

void hh_reg_join_cb_func(void (* join_req)(struct hh_dev_info *, enum hh_join_stage))
{
	sw_dev_join_cb = join_req;
}

/**
 * \brief Send an ACK for new device join net
 */
int32_t hh_join_ack(struct hh_dev_info *info, bool accept)
{
	struct hh_prot p;

	memset(&p, 0, sizeof(struct hh_prot));
	p.ctl = HH_CONN_CTRL;
	p.counter = hh_get_counter();
	p.subPkt_count = 1;
	p.subPkt_index = 1;
	p.dataLen = 4;
	p.data[0] = 0x24;
	p.data[1] = accept ? 0x01 : 0x00;
	*((uint16_t *) (&(p.data[2]))) = info->saddr;

    HHP_DEBUG("[Control] Ack device(%02X%02X) join %s", p.data[2], p.data[3], (accept)?"Accept":"Reject");
	hh_send_data(&p);

	return HHP_STATUS_OK;
}

enum
{
	PROT_SEQ_HEAD1,
	PROT_SEQ_HEAD2,
	PROT_SEQ_DATA
};

static uint8_t    prot_seq;       /* Sequence */
static uint8_t    usart_rx_len;   /* Receive Data Length */

/**
 * \brief Reset USART receive buffer
 */
void hh_usart_reset_buf(void)
{
    usart_rx_len = 0;
	prot_seq = PROT_SEQ_HEAD1;
	snmp_incoming_msg(NULL, 0);
}

/**
 * \brief Read data from Serail Port
 */
enum hh_msg_type hh_usart_read(uint8_t data)
{
    if(usart_rx_len > sizeof(struct hh_prot)) {
		goto Reset;
	}

	switch(prot_seq) {
	case PROT_SEQ_HEAD1:
		if(data != 0xF0) {
			goto Reset;
		}
		usart_rx_len = 0;
		prot_seq = PROT_SEQ_HEAD2;
		break;

	case PROT_SEQ_HEAD2:
		if(data != 0xA5) {
			goto Reset;
		}
		prot_seq = PROT_SEQ_DATA;
		break;

	case PROT_SEQ_DATA:
		((uint8_t *)(&recv_pkg))[usart_rx_len++] = data;
		if (usart_rx_len == sizeof(struct hh_prot)) {
			usart_rx_len = 0;
			prot_seq = PROT_SEQ_HEAD1;
			/* Process Message */
			return _hh_process_incoming_pkg();
		}
		break;

	default:
		goto Reset;
	}

	return HH_MSG_DATA;

Reset:
    HHP_DEBUG("Invalid Data Received");
	hh_usart_reset_buf();
	return HH_MSG_DATA;
}

/**
 * \brief Process incoming data
 */
static enum hh_msg_type _hh_process_incoming_pkg()
{
	uint16_t crc;
	struct hh_dev_info info;
    int i = 0;

	crc = crc16_data2(((uint8_t *)&recv_pkg), HH_PROT_DATACRC_LEN, 0);
    HHP_DEBUG_HEXSTRING(i, ((uint8_t *)&recv_pkg), sizeof(struct hh_prot));

	if (crc != recv_pkg.crc) {
        HHP_ERROR("CRC Error");
		return HH_MSG_DATA;
	}
	if (recv_pkg.ctl == HH_CONN_CTRL) {
		if (recv_pkg.data[0] == 0x23 && recv_pkg.data[1] == 0x01) {
			status = HH_CONN_CTRL;
		}
		else if (recv_pkg.data[0] == 0x24 && recv_pkg.dataLen == 0x17) {
			struct hh_prot_dev_info *dinfo =
				(struct hh_prot_dev_info *)(&recv_pkg.data[1]);
			info.saddr = dinfo->saddr;
			info.sw_ver = dinfo->sw_ver;
			info.hw_ver = dinfo->hw_ver;
			memcpy(info.mac, dinfo->mac, 8);
			info.type = dinfo->type;
			if (sw_dev_join_cb) {
				sw_dev_join_cb(&info, HH_JOIN_REQUEST);
			}
		} else if (recv_pkg.data[0] == 0x24 && recv_pkg.data[1] == 0x66 && recv_pkg.dataLen == 0x04) {
			info.saddr = *((uint16_t *) (&recv_pkg.data[2]));
			if (sw_dev_join_cb) {
				sw_dev_join_cb(&info, HH_JOIN_FINISHED);
			}
		}
		return HH_MSG_CTRL;
	} else if  (recv_pkg.ctl == HH_CONN_ONLINE) {
        if (recv_pkg.subPkt_index == 1) {
            snmp_incoming_msg(NULL, 0);
        }
		snmp_incoming_msg(recv_pkg.data + 4, recv_pkg.data[3]);
		if (recv_pkg.subPkt_count > recv_pkg.subPkt_index) {
			return HH_MSG_DATA;
		}
		switch (snmp_parse_type()) {
		case GET_RESPONSE:
			return HH_MSG_GET_RESP;
		case SET_REQUEST:
			return HH_MSG_SET_RESP;
		case TRAP_V1:
		case TRAP_V2:
			return HH_MSG_TRAP;
		case TRAP_INFORM:
			return HH_MSG_TRAP;
        case HH_MSG_GATEWAY:
            return HH_MSG_GATEWAY;
		default:
			break;
		}
		return HH_MSG_DATA;
	}
	return HH_MSG_DATA;
}

int32_t hh_dev_send_msg(uint16_t dev)
{
    uint8_t *msg;
    uint32_t len;
    uint32_t index = 0;
    struct hh_prot pkg;
    uint16_t crc;
    uint8_t i;

    extern void hh_msg_get_send_info(uint8_t **data, uint32_t *len);

    /* Get SNMP message info */
    hh_msg_get_send_info(&msg, &len);
    pkg.ctl = HH_CONN_ONLINE;
    pkg.counter = hh_get_counter();
    pkg.subPkt_count = (len / 50) + ((len % 50) ? 1 : 0);
    pkg.subPkt_index = 0x01;

    do {
        if (len - index > 50) {
            pkg.dataLen = 50 + 4;
        } else {
            pkg.dataLen = len - index + 4;
        }
        pkg.data[0] = 0x41;
        pkg.data[1] = dev >> 8;
        pkg.data[2] = dev & 0xFF;
        pkg.data[3] = pkg.dataLen - 4;
        memcpy(pkg.data + 4, msg + index, pkg.dataLen);


        index += pkg.dataLen - 4;
        crc = crc16_data2(((uint8_t *)&pkg), HH_PROT_DATACRC_LEN, 0);
        pkg.crc = crc;

        hw_usart_send(0xF0);
        hw_usart_send(0xA5);
        for (i = 0; i < sizeof(struct hh_prot); i++) {
            hw_usart_send(((uint8_t *)&pkg)[i]);
        }
        HHP_DEBUG_HEXSTRING(i, ((uint8_t *)&pkg), sizeof(struct hh_prot));

        pkg.subPkt_index++;
        memset(pkg.data, 0x00, MAX_DATA_LENGTH);
    } while (index < len);


    return HHP_STATUS_OK;
}

void hh_get_gateway_info(uint8_t **data, uint32_t *len)
{
    hh_msg_resp_buffer(data, len);
}