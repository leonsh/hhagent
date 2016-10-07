#ifndef _HHP_H_
#define _HHP_H_
#include <stdint.h>
#include <stdbool.h>
#include <compiler.h>
#include "hhp_error_code.h"
#include "hhp_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 设备状态
 */
enum hh_conn_status {
    HH_CONN_ONLINE  = 0x00,  /*!< 设备在线，可以正常通讯 */
    HH_CONN_CTRL    = 0xC0,  /*!< 控制状态，仅收发组网报文 */
    HH_CONN_OFFLINE = 0xFF   /*!< 设备离线 */
};

/**
 * 设备注册状态
 */
enum hh_join_stage {
    HH_JOIN_REQUEST,        /*! 注册请求 */
    HH_JOIN_FINISHED        /*! 注册完成 */
};
enum hh_msg_type {
    HH_MSG_DATA,      /*!< 数据接收中，未完整报文 */
    HH_MSG_CTRL,      /*!< 控制报文 */
    HH_MSG_SET_RESP,  /*!< 修改设备配置结果 */
    HH_MSG_GET_RESP,  /*!< 查询设备配置结果 */
    HH_MSG_TRAP,      /*!< 设备主动上报消息 */
    HH_MSG_INFORM,    /*!< 设备主动上报消息，需要应答 */
    HH_MSG_GATEWAY    /*!< 网关类型设备返回的查询结果 */
};

/**
 * \berif PDU配置项名称
 *
 * Object ID通常指设备属性ID(名称),设备可能存在多种OID
 */
enum hh_oid_name {
    HH_OID_SWITCH,            /*!< 开关 */
    HH_OID_SWITCH_STATUS,     /*!< 开关状态 */
    HH_OID_ION,
    HH_OID_ION_STATUS,
    HH_OID_ION_ELEC
};

/**
 * \berif PDU配置项数据类型
 */
enum hh_oid_type {
    HH_OID_INTEGER       = 0x02,   /*!< Integer 类型, 必须为 0x02 */
    HH_OID_OCTET_STRING  = 0x04    /*!< String 类型, 必须为 0x04 */
};

/**
* \berif 接收到的数据类型
 */
/*
enum hh_recv_type {
    HH_RECV_COMING             = 0xFF,
    HH_RECV_CTRL_SWITCH_STATUS = 0x23,
    HH_RECV_CTRL_JOIN_REQ      = 0x25,
    HH_RECV_DEV_INFORM         = 0xD0
};
*/

/**
 * 设备类型
 */
enum hh_dev_type {
    HH_DEV_SWITCH = 0x00010000,   /*!< 开关 */
    HH_DEV_ION         = 0x00010000    /*!< ION */
};

struct hh_pdu {
    uint32_t      reqid;
    uint32_t      err;
    struct hh_pdu_var   *var;
};

/**
 * 设备信息
 */
struct hh_dev_info {
    uint16_t            saddr;   /*! 短地址 */
    uint8_t             mac[8];  /*!< MAC 地址 */
    enum hh_dev_type    type;    /*!< 设备类型 */
    enum hh_conn_status status;  /*!< 设备状态 */
    uint16_t            sw_ver;  /*!< 软件版本 */
    uint16_t            hw_ver;  /*!< 硬件版本 */
    uint32_t            vendor;  /*!< 厂商ID */
};

/**
 * \berif 设备配置信息
 */
struct msg_entry {
	enum hh_dev_type   dev_type;
    enum hh_oid_name   oid_name;
    uint8_t            oid_len;
    uint8_t            set_len;
    uint8_t            index;
    uint8_t            oid[14];
	uint8_t            oid_type;
};

struct hh_oid_entry {
  	enum hh_dev_type   dev_type;
    enum hh_oid_name   oid_name;
    enum hh_oid_type   oid_type;
    int32_t            index;
    union {
      int32_t int_val;
    }val;
};
/** \brief 切换Zigbee模块工作状态
 *
 * 切换HomeHeart Zigbee模块工作状态，\ref HH_CONN_ONLINE 在线通讯状态，模块
 * 可以与子设备正常通讯。\ref HH_CONN_JOINING 组网状态，模块可以接收/应答新
 * 设备入网请求。
 *
 * \note 在切换到组网状态前，需要先调用 \ref hh_dev_reg_cb_report() 函数，注册
 * 入网请求回调函数。在组网状态下时，模块无法与已注册的设备进行通讯。组网状态
 * 持续时间建议60秒。
 *
 * \param status 工作状态
 */
int32_t hh_switch_status(enum hh_conn_status status);

/**
 * \brief 注册串口发送函数
 */
void hh_usart_reg_write_func(void (* usart_send)(uint8_t));

/**
 * \brief 串口接收函数
 *
 * 接收串口数据，用户程序判断串口有数据到达后，调用该函数来接收数据。
 *
 * \param data 串口数据
 *
 * \return
 */
void hh_usart_write(uint8_t data);

/**
 * \brief 读取串口数据
 *
 * 接收Zigbee串口数据，并解析接收到的数据报文, 用户程序判断串口有数据到达后，
 * 调用该函数来接收数据。
 *
 * \param data 串口数据
 *
 * \return 接收到的报文数据类型
 * \retval HH_MSG_DATA 报文数据，报文还未接收完成，需要继续接收报文后续数据
 * \retval HH_MSG_CTRL 控制报文
 */
enum hh_msg_type hh_usart_read(uint8_t data);

/**
 * \brief 重置串口接收缓冲区
 *
 * 通常串口在接收一个有效的数据包时，接收字节之间的间隔应小于一定的间隔，
 * 如果接收字节间隔过长，并且前一个数据帧未接收完成，则可能发生串口数据丢失
 * 。应用程序可以在判断接收超时后调用该函数重置协议栈的串口接收缓冲区，建议
 * 应用程序超时判断设置为100ms。
 */
void hh_usart_reset_buf(void);

/**
 * \brief 注册新设备加入回调函数
 */
void hh_reg_join_cb_func(void (* join_req)(struct hh_dev_info *, enum hh_join_stage));

/**
 * \brief 应答新设备加入请求
 */
int32_t hh_join_ack(struct hh_dev_info *info, bool accept);

/**
 * \brief 注册设备主动上报回调函数
 */
void hh_dev_reg_cb_report(struct hh_dev_info *info);

/**
 * \brief 获取PDU报文发送设备的ID
 *
 * 该函数仅在\ref hh_usart_read 接收到的报文数据类型不等于 \ref HH_MSG_DATA时
 * 有效。
 *
 * \param[in] 设备ID指针 \ref saddr
 *
 * \return 获取结果
 * \retval HHP_STATUS_OK 正确获取到到子设备ID
 * \retval HHP_NO_RESOURCE 未接收到子设备消息或消息格式错误
 */
int32_t hh_msg_get_dev_id(uint16_t *dev);

/**
 * \brief 获取PDU报文中的配置项信息
 *
 * 子设备返回的PDU报文中可存在多个配置项，调用该函数可返回下一个配置项的信
 * 息，当返回\ref HHP_NO_RESOURCE 时表明所有配置项已轮询完毕。通常一个配置项
 * 信息包含：名称，类型，索引值，配置值。
 *
 * \note 该函数仅在\ref hh_usart_read 接收到的报文数据类型不等于
 * \ref HH_MSG_DATA时有效。
 *
 * \param[out] entry 配置项信息,包含名称,类型,索引,值
 *
 * \return 获取结果
 * \retval HHP_STATUS_OK 正确获取到到子设备ID
 * \retval HHP_NO_RESOURCE 未接收到子设备消息或消息格式错误
 */
int32_t hh_msg_get_next_item(struct hh_oid_entry *entry);

/**
 * \brief 创建Set PDU报文中的配置项信息。
 *
 * 创建一个Set PDU报文，一个Set PDU报文中可存在多个配置项。该函数并不会发送
 * Set PDU报文到设备。用户需要继续调用\ref hh_dev_send_msg来完成发送
 * Set PDU报文。
 *
 * \param[out] entry 配置项信息,包含名称,类型,索引,值
 *
 * \return 操作结果
 * \retval HHP_STATUS_OK 正确创建Set PDU报文。
 */
int32_t hh_msg_set_item(struct hh_oid_entry *entry, uint8_t len);

/**
 * \brief 发送PDU报文到设备
 *
 * 发送PDU报文到设备，发送之前应先调用\ref hh_msg_set_item创建Set PDU
 *
 * \param[int] dev 设备ID
 *
 * \return 操作结果
 * \retval HHP_STATUS_OK 正确发送PDU报文。
 */
int32_t hh_dev_send_msg(uint16_t dev);

/**
 * \brief 获取Gateway设备返回的查询信息
 *
 * \note 该函数仅在\ref hh_usart_read 接收到的报文数据类型等于\ref HH_MSG_GATEWAY
 * 时有效，应用程序需要首先调用\ref hh_msg_gateway_query_detail, \ref hh_msg_gateway_query_devlist
 * \ref hh_msg_gateway_query_taglist发送相应的查询报文。
 *
 * \param[out] data 数据指针
 * \param[out] len  数据长度
 *
 */
void hh_get_gateway_info(uint8_t **data, uint32_t *len);

/**
 * \brief 发送查询消息到Gateway设备，查询命令为Detail
 *
 * 该函数并不会立即发送查询报文到设备。用户需要继续调用\ref hh_dev_send_msg
 * 来完成发送.
 *
 */
void hh_msg_gateway_query_detail(void);

/**
 * \brief 发送查询消息到Gateway设备，查询命令为DevList
 *
 * 该函数并不会立即发送查询报文到设备。用户需要继续调用\ref hh_dev_send_msg
 * 来完成发送.
 *
 */
void hh_msg_gateway_query_devlist(void);

/**
 * \brief 发送查询消息到Gateway设备，查询命令为TagList
 *
 * 该函数并不会立即发送查询报文到设备。用户需要继续调用\ref hh_dev_send_msg
 * 来完成发送.
 *
 */
void hh_msg_gateway_query_taglist(void);

/**
 * \page hhpdoc_qs_basic_use_case 快速入门指南
 *
 * HomeHeart Protocol SDK 协议栈(以下简称SDK或HHPSDK)是家联IOT(Internet of
 * Things)系统的中间价软件包，主要目的为第三方合作伙伴提供接入并使用家联物
 * 联网智能设备的软件控制接口。SDK使用标准C语言实现，并提供了一套易于使用的
 * API接口。SDK软件包可以使用在任任意微控制器或微处理器平台。
 *
 * \image html coordinator.jpg
 *
 * HomeHeart Protocol SDK (hhpsdk) 使用步骤:
 * - 初始化 SDK
 * - 重启 Zigbee Coordinator (协调器) 模块
 * - 初始化 Zigbee Coordinator (协调器) 模块
 * - 切换 Zigbee Coordinator 模块工作状态
 * - 注册新设备入网
 * - 获取设备信息
 * - 修改设备参数
 * - 接收设备通知信息
 *
 * \section hhpdoc_qs_sdk_init 初始化 SDK
 * SDK使用过程中需要调用系统外设函数来完成串口的数据收发，以及调用用户注册
 * 的回调函数完成设备控制的相关功能。SDK协议栈初始化，主要完成向SDK注册回
 * 调函数，并开始接收协调器数据。
 *
 * \subsection hhpdoc_qs_sdk_init_reg_usart_send 注册串口发送函数
 * 协议栈在使用过程中需要使用系统的串口来发送数据到zigbee协调器，应用程序需要调用
 * \ref hh_usart_reg_write_func实现串口发送函数的注册。
 * \snippet main.c sdk_serial_setup
 *
 * \subsection hhpdoc_qs_sdk_init_reg_dev_cb 注册设备主动上报回调函数
 * 通常在设备的注册过程中，会主动上报注册请求报文，SDK在收到注册请求报文后，会
 * 调用应用层注册的回调函数通知应用程序.\ref hh_reg_join_cb_func
 * \snippet main.c sdk_reg_cb_setup
 * 
 * \section hhpdoc_qs_usart_read 串口数据读取
 * 应用程序在接收到串口接收到数据后，需要调用\ref hh_usart_read函数来使用SDK解析
 * 收到的数据，根据函数返回的数据类型，应用程序可以得知所收的数据包是否已完成。
 * - 当返回类型为\ref HH_MSG_DATA 时，表明数据包还未接收完毕，应用程序需要继续等待
 * 串口接收数据。
 * - 当返回类型为\ref HH_MSG_CTRL 时，表明收到的为控制报文。
 * - 当返回类型为\ref HH_MSG_TRAP \ref HH_MSG_SET_RESP \ref HH_MSG_GET_RESP
 * \ref HH_MSG_INFORM 时，表明收到的数据为设备主动上报报文，应用程序
 * 可以调用\ref hh_msg_get_next_item来获取配置项信息。
 * - 当返回类型为\ref HH_MSG_GATEWAY时，表明收到的数据为网关类型设备所返回的查询
 * 结果。
 * 
 * \section hhpdoc_qs_zigbee_control 组网控制
 * 可以通过调用\ref hh_switch_status 来切换协调器当前状态，\ref HH_CONN_CTRL 仅
 * 处理组网相关的数据报文。\ref HH_CONN_ONLINE 可以接收子设备发送的数据报文。
 *
 * 
 * \section hhpdoc_qs_dev_control 设备控制
 * 调用\ref hh_msg_set_item 控制设备，该函数并不会立即发送数据到设备，应用程序
 * 需要继续调用\ref hh_dev_send_msg 来完成控制信息的发送。
 *
 *
 * \section hhpdoc_qs_gateway 网关类型设备的控制
 * 网关类型设备由于含有不同类型的子设备，所以其控制项存在多种组合，应用程序可以
 * 通过发送网关查询报文来获取其相应的设置项\ref hh_msg_gateway_query_detail 
 * \ref hh_msg_gateway_query_devlist \ref hh_msg_gateway_query_taglist
 * 并通过\ref hh_get_gateway_info 来获取网关设备返回的信息。
 *
 *
 * \section hhpdoc_qs_zigbee_fw_upgrade 协调器固件烧录
 * Zigbee协调器在使用前，需要先烧录固件程序。
 * - 将协调器模块模块连接到电脑USB口
 * - 将Zigbee(CC2530)烧录器连接到协调器模块
 * - 将fw/CC2530SB.hex bootloader通过烧录器烧写到cc2530
 * - 断开烧录器（后续步骤不再需要烧录器）
 * - 主程序通过SerialBootTool.exe烧写的cc2530, 串口参数为115200, 8N1。需要注意在
 * 烧写前需要按一下协调器模块的Reset按钮，然后点击Load Image（重起后的2秒内）按
 * 钮进行串口烧录。
 *
 * \section hhpdoc_qs_windows_app Win32测试源代码使用
 * 下载codeblock mingw-nosetup http://www.codeblocks.org/downloads, 打开
 * hhp.workspace，编译homeheartprotocol工程即可得到Win32 Console应用程序
 */

#ifdef __cplusplus
}
#endif

#endif // _HHP_H_
