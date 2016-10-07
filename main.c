#include <stdio.h>
#include <stdlib.h>
#include <hhp.h>
#include <pthread.h>
#include <windows.h>

struct hh_dev_info dev_info;
#define DEV_ID(id) (is_big_endian() ? id :  (((id & 0xFF) << 8) | (id >> 8)))

/**
 * \brief Display the user menu on the terminal.
 */
static void display_menu(void)
{
	printf("\n\rMenu :\n\r"
			"------\r");

	printf("  o: 通讯模式\n\r"
            "  c: 组网模式\n\r"
            "  s: 切换ION开关状态\n\r"
            "  g: 切换Switch开关状态\n\r"
			"  h: Display this menu again\n\r\r");
}

void usart_write(uint8_t data)
{
    writeToSerialPort(data);
}

void demo_join_req_cb(struct hh_dev_info *info, enum hh_join_stage stage)
{
    if (stage == HH_JOIN_REQUEST) {
        printf ("New Device Join Request: \r\n");
    } else if (stage == HH_JOIN_FINISHED) {
        printf ("New Device Join Finished\r\n");
        return;
    }
	memcpy(&dev_info, info, sizeof(struct hh_dev_info));
    printf("Device Short Address: %04X\r\n", dev_info.saddr);
    printf("Device Full Address: %02X%02X%02X%02X%02X%02X%02X%02X\r\n",
           dev_info.mac[0], dev_info.mac[1], dev_info.mac[2], dev_info.mac[3],
           dev_info.mac[4], dev_info.mac[5], dev_info.mac[6], dev_info.mac[7] );
    printf("Device Type: %08X\r\n", dev_info.type);
    printf("Device Hardware Version: %04X\r\n", dev_info.hw_ver);
    printf("Device Software Version: %04X\r\n", dev_info.sw_ver);
    printf("Device VendorID: %08X\r\n", dev_info.vendor);

    printf("Send Allow Join\r\n");
    hh_join_ack(&dev_info, true);
}

void sdk_init(void)
{
//! [sdk_serial_setup]
  /* User Application need to implement USART send function */
  extern void usart_write(uint8_t data);
  //! [sdk_serial_setup_1]
  hh_usart_reg_write_func(usart_write);
  //! [sdk_serial_setup_1]
//! [sdk_serial_setup]
//! [sdk_reg_cb_setup]
    hh_reg_join_cb_func(demo_join_req_cb);
//! [sdk_reg_cb_setup]
}
int ion_state = 0;
void toggle_ion(void)
{
    struct hh_oid_entry entry;

    entry.index = 0;
    entry.oid_name = HH_OID_ION;
    entry.oid_type = HH_OID_INTEGER;
    entry.val.int_val = ion_state;
    ion_state = !ion_state;

    hh_msg_set_item(&entry, 1);
    hh_dev_send_msg(DEV_ID(dev_info.saddr));
}
void toggle_switch(void)
{
   uint8_t oid[13] = {0x2b, 6, 1, 4, 1, 0x82, 0xc3, 0x7b, 1, 0x84,0x59, 1, 0};

    ion_state = !ion_state;
    hh_msg_set_item_raw(oid, 13, ion_state);
    hh_dev_send_msg(DEV_ID(dev_info.saddr));
}
void menu_task(void)
{
    char uc_key;
    /* Display menu. */
	display_menu();

    while (1) {
        scanf("%c", (char *)&uc_key);

            switch (uc_key) {
		case 'h':
			display_menu();
			break;

		case 'o':
		    hh_switch_status(HH_CONN_ONLINE);
			break;
        case 'c':
		    hh_switch_status(HH_CONN_CTRL);
			break;
        case 's':
		    toggle_ion();
			break;
         case 'g':
		    toggle_switch();
			break;
		default:
			/* Set configuration #n. */
			break;
		}
	}
}
int main()
{

    uint8_t data;
    int32_t rst;
    pthread_t thread1;

    sdk_init();

    if (openSerial() == 0) {
        printf("串口打开成功\r\n");
    } else {
        printf("串口打开失败\r\n");
    }
    pthread_create (&thread1, NULL, (void *) &menu_task, NULL);

	while (1) {
		if (readFromSerialPort(&data) == 1) {
            printf("%02X", data);
            fflush(stdout);
            hh_usart_read(data);
		}
		Sleep(1);
	}

    return 0;
}

