#include <unity.h>

void test_crc16_data2_should_ok(void);
void test_hh_switch_status_should_send_data(void);
void test_hh_send_data(void);
void test_hh_receive_data(void);
void test_hh_device_join(void);
void test_recevice_v2_trap(void);
void test_recv_trap_with_multi_entries(void);
void test_build_snmp_setpdu(void);
void test_build_snmp_getpdu(void);

void test_parse_gateway_details_msg(void);
void test_parse_gateway_devList_msg(void);
void test_parse_gateway_tagList_msg(void);

void test_gateway_query_datail_msg(void);
void test_gateway_query_devlist_msg(void);
void test_gateway_query_taglist_msg(void);

void test_build_snmp_setpdu_raw(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_crc16_data2_should_ok);
    RUN_TEST(test_hh_switch_status_should_send_data);
    RUN_TEST(test_hh_send_data);
    RUN_TEST(test_hh_receive_data);
    RUN_TEST(test_hh_device_join);
    RUN_TEST(test_recevice_v2_trap);
    RUN_TEST(test_recv_trap_with_multi_entries);
    RUN_TEST(test_build_snmp_setpdu);
    RUN_TEST(test_build_snmp_getpdu);
    
    RUN_TEST(test_parse_gateway_details_msg);
    RUN_TEST(test_parse_gateway_devList_msg);
    RUN_TEST(test_parse_gateway_tagList_msg);

    RUN_TEST(test_gateway_query_datail_msg);
    RUN_TEST(test_gateway_query_devlist_msg);
    RUN_TEST(test_gateway_query_taglist_msg);
    RUN_TEST(test_build_snmp_setpdu_raw);

    return UNITY_END();
}
