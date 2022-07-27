/*****************************************************************************
Copyright:      2016-2026, ISVISION (Hangzhou) Tech. Co., Ltd.
Filename:       isv_proto.h
Author:         yanke@isv-tech.com
Description:    communication protocol for isvision
Others:         none
Date:           2020/04/16
History: 
1. Date:        2020/04/16
   Author:      yanke@isv-tech.com
   Modification:original edition
*****************************************************************************/

#ifndef _ISV_PROTOCOL_H_
#define _ISV_PROTOCOL_H_

#include <rtthread.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* fixed operation code1 */
#define OP1_INVALID     (-1)
#define OP1_MAX         0xC8    // op1 allow range
#define VALID_OP1(op1)  ((op1) >= 0 && (op1) < OP1_MAX)
#define OP1_BROD_FIND   0xA0
#define OP1_DEV_INFO    0xA1
//#define OP1_EEPROM_GET  0xA2    // get lower CPU's EEPROM value
#define OP1_EEPROM_GET  0xA2    // get lower CPU's EEPROM value
#define OP1_EEPROM_SET  0xA3    // set lower CPU's EEPROM value
#define OP1_LOG_L2U     0xA5    // log report from lower CPU to upper CPU
#define OP1_DFX_REQ     0xA6    // request lower CPU's DFx
#define OP1_HRT_L2U     0xA7    // heartbeat request from lower CPU to upper
#define OP1_HRT_U2L     0xA8    // heartbeat request from upper CPU to lower
#define OP1_TIME_L2U    0xA9    // timestamp from lower CPU to upper CPU
#define OP1_TIME_U2L    0xAA    // set lower CPU's time
#define OP1_UPG_REQ     0xAB    // upgrade lower CPU's system
#define OP1_SERL_EN     0xAE    // enable/disable lower CPU's serial print out
#define OP1_FIND_ACK    0xB0
#define OP1_VER_REQ     0xB2    // request lower CPU's system version
#define OP1_UPG_SYNC    0xB3    // request lower CPU's upgrade sub-frame size
#define OP1_SYS_REQ     0xB4    // request lower CPU's system operation
#define OP1_COMM_END    0xC0

/* fixed operation code2 */
#define OP2_GET_INFO    0x01
#define OP2_SET_IP_MASK 0x02
#define OP2_SET_DEV_SN  0x03
#define OP2_SET_CAM_SN  0x04
#define OP2_BASE_INFO   0x01
#define OP2_UPG_FIRM    0x01    // upgrade lower CPU's firmware
#define OP2_UPG_INI     0x02    // upgrade lower CPU's INI config file
#define OP2_LOG_ERROR   0x00    // error log report from lower CPU to upper CPU
#define OP2_LOG_DFX     0x01    // DFX log report from lower CPU to upper CPU
#define OP2_VER_APP     0x00    // get application version
#define OP2_VER_PLAT    0x01    // get platform version
#define OP2_VER_FPGA    0x02    // get FPGA version
#define OP2_SYS_RESET   0x01    // reset(reboot) lower CPU
#define OP2_SYS_WAKE    0x02    // wakeup lower CPU
#define OP2_SYS_SLEEP   0x03    // sleep lower CPU

///* common error code */
typedef enum {
	SUCCEED,
	ERR_OPT2,
	ERR_PAYLOAD_OPT,
	ERR_DATA,
	ERR_EXEC,
	ERR_CRC,
}pro_ret;

#pragma pack(1)

/* protocol frame format: */
/* head */
/* payload */
/* padding zeros */
/* tail */
#define PRO_DATA_ALIGN          4
#define PRO_MAGIC_HEAD          0x5A5A5A5A
#define PRO_VERSION_2           2
#define PRO_EN_CRC32            1
/* all elements in this structure is stored in network(big) endian */
typedef struct proto_head {
    unsigned int magic;         // protocol head magic number, fixed to 0x5A5A5A5A
    unsigned int version;       // protocol version, support version 2 only
    unsigned int frame_len;     // current frame length, including protocol head, payload in this frame and protocol tail

    /* elements in one 4byte unit is arranged by little endian */
    unsigned char resv1[3];     // reserved for future use
    unsigned char resv2 :7,     // reserved for future use
                  crc :1;       // bit31 for 4byte(int) data unit, ARM CPU is little endian

    unsigned short frame_cur;   // current frame number
    unsigned short frame_sum;   // sum frame number

    unsigned int paylen;        // total payload length including all sub frames and pads

    unsigned char prod_ver;     // product version number
    unsigned char prod_type;    // product type 
    unsigned char padsize;      // payload zero padding size in this frame
    unsigned char frame_seq;    // frame sequence number, used to indentify multi-frames

    unsigned short opcode2;     // operation code 2
    unsigned char  opcode1;     // operation code 1
    unsigned char  errcode;     // error code
}ProHead;
#define PRO_HEAD_LEN    sizeof(ProHead)

/* all elements in this structure is stored in network(big) endian */
#define PRO_MAGIC_TAIL          0xA5A5A5A5
typedef struct proto_tail {
    unsigned int crc32;         // CRC32 checksum, from proto_head->version to the last byte of payload
    unsigned int magic;         // protocol tail magic number, fixed to 0xA5A5A5A5
}ProTail;
#define PRO_TAIL_LEN    sizeof(ProTail)

#define PRO_MAX_PAYLEN          (200 - PRO_MIN_SIZE)
#define PRO_MIN_SIZE            (PRO_HEAD_LEN + PRO_TAIL_LEN)
#define PRO_MAX_SIZE            (PRO_MAX_PAYLEN + PRO_MIN_SIZE)
#define PRO_FRAME_LEN(paylen)   (PRO_MIN_SIZE + (paylen))

typedef struct {
    int len;            // frame data buffer length
    void *data;         // frame data buffer including protocol head, payload, tail
}ProFrame;
#define PRO_HEAD(f)     ((ProHead *)((f)->data))
#define PRO_TAIL(f)     ((ProTail *)((char *)(f)->data + (f)->len - PRO_TAIL_LEN))
#define PRO_PAYLOAD(f)  ((char *)(f)->data + PRO_HEAD_LEN)
#define PRO_PAYSIZE(f)  ((f)->len - PRO_HEAD_LEN - PRO_TAIL_LEN - PRO_HEAD(f)->padsize)

#pragma pack()

typedef int (* RSCallback)(void *args, void *buff, int size);
typedef struct {
    int prod_type;      // channel's product type
    int prod_ver;       // channel's product version

    void *args;         // callback args, will be passed into callback functions
    RSCallback send;    // frame send callback function
    char sendbuff[PRO_MAX_SIZE];   //receive buffer
	char recvbuff[PRO_MAX_SIZE];   //send buffer
	unsigned recv_size;            //receive size
	unsigned send_size;            //send size
    rt_mutex_t recv_mutex;    //receive buffer mutex lock
    unsigned send_cnt;  // sended frame count, including sub-frame
    unsigned recv_cnt;  // received frame count, including sub-frame and err-frame
	rt_list_t list;     // channel's list node
}ProChan;

int proChanInit(ProChan *chan, int prodtype, int prodver, RSCallback send, void *args);
int proChanExit(ProChan *chan);

/* @Brief: parsing protocol frame in specified buffer
 * @Param: *buff: protocol frame buffer
 *         len: buffer len
 * @RET:   succ: 0
 *         fail: negative error number */
int proParseFrame(ProChan *chan, void *buff, int len);

/* @Brief: packing protocol frame and send 
 * @Param: *payload: payload buffer, pass NULL if no need to send payload
 *         size: payload size, pass 0 if no need to send payload
 *         fr_cur: current frame number
 *         fr_sum: total frame number
 *         fr_seq: frame sequence number
 *         op1: frame operation code 1
 *         op2: frame operation code 2
 *         err: frame error code
 * @RET:   succ: 0
 *         fail: negative error number */
int proSendOne(ProChan *chan, void *payload, int size, int fr_cur, int fr_sum, int fr_seq,
               int op1, int op2, int err);

#define proSendData(chan, pay, len, op1, op2, err)      \
    proSendOne(chan, pay, len, 1, 1, 0, op1, op2, err)

#define proSendNull(chan, op1, op2, err)                \
    proSendOne(chan, NULL, 0, 1, 1, 0, op1, op2, err)

#define proSendAckData(chan, req, payload, size, err)        \
    proSendOne(chan, payload, size, 1, 1, req->frame_seq, req->opcode1, req->opcode2, err)

#define proSendAckNull(chan, req, err)                   \
    proSendOne(chan, NULL, 0, 1, 1, req->frame_seq, req->opcode1, req->opcode2, err)

int proFindHead(ProChan *chan, void *buff, int len, ProFrame *frame);
int proCheckFrame(ProChan *chan, ProFrame *frame);

typedef struct  {
	char *buf;                /*发送数据缓冲buffer*/
	int size;                 /*发送数据长度*/     
	unsigned short op2;       /*需要返回的opcode2*/
	unsigned char op1;        /*需要返回的opcode1*/
}snd_msg;

typedef struct {
	unsigned short opcode2;   /*opcode2 操作码*/
	unsigned short frame_cur; /*当前协议帧数*/
    unsigned short frame_sum; /*总的协议帧数*/
	char *buf;                /*接收数据缓冲buffer*/
	int size;                 /*接收数据长度*/
}rev_msg;

typedef pro_ret (*pro_fun)(snd_msg *snd, rev_msg *rev);

struct operation {
	pro_fun fun;              /*操作码对应的处理函数*/
};

#define EXPORT_OPCODE(opt, fun) \
int export_opcode_##fun(void) { \
	register_opcode_operation(opt, fun); \
	return 0; \
} \
SYSINIT_EXPORT(export_opcode_##fun)

int operation_init(void);
void register_opcode_operation(int opcode1, pro_fun fun);
void unregister_opcode(int opcode1);
struct operation* find_operation(int opcode1);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

