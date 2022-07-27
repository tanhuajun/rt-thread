
#define LOG_TAG   "protocol"
#define LOG_LVL   LOG_LVL_INFO

#include "protocol.h"
#include <ulog.h>
#include <sys/socket.h>
#include "hash.h"
#include <rtthread.h>

#define OFFSET(T, M)        ((unsigned long)(&(((T *)(0))->M)))
#define round_up(x, y)      ((((x) + (y) - 1) / (y)) * (y))

/* used by crc32 calculate */
static const unsigned int table[256] = {
0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d, };

int proFindHead(ProChan *chan, void *buff, int len, ProFrame *frame)
{
    /* if we want to check more header tags, just add other elements here */
    unsigned int magic = 0;     // should be equals to proto_head->magic
    unsigned int version = 0;   // should be equals to proto_head->version
    unsigned int frame_len = 0;
    unsigned char *data = (unsigned char *)buff;
#define TAG_LEN     (sizeof(magic) + sizeof(version) + sizeof(frame_len))

    /* locate protocol header magic number */
    for (; len + TAG_LEN >= PRO_MIN_SIZE; data++, len--) {
        /* shift one byte, assumes local machine is little endian (certainly) */
        magic = (magic << 8) | (version >> 24);
        version = (version << 8) | (frame_len >> 24);
        frame_len = (frame_len << 8);

        rt_memcpy(&frame_len, data, 1);

        /* PR_DBG("[0x%08x %08x %08x]", magic, version, frame_len); */
        /* head magic and version is turned to little endian by byte shifting */
        if ( (magic   != PRO_MAGIC_HEAD) ||
             (version != PRO_VERSION_2 ) ||
             (frame_len < PRO_MIN_SIZE ) ) {
            continue;
        }

        /* got an valid frame now, breakout */
        log_d("---------- New frame, frame length[%d]", frame_len);
        break;
    }

    if (len + TAG_LEN < PRO_MIN_SIZE || len + TAG_LEN < frame_len)
        return -1;

    frame->len = frame_len;
    frame->data = data - TAG_LEN + 1;
    return 0;
}

static unsigned int proCrc32(ProHead *head, char *data, int datasize, int padsize)
{
    unsigned int crc = 0xFFFFFFFF;
    unsigned char sn = 0;
    char *pbuf = NULL;
    int len = 0;

    /* protocol head, start from proto_head->version */
    pbuf = (char *)&head->version;
    len  = sizeof(*head) - OFFSET(ProHead, version);
    while (len--) {
        sn = (unsigned char)crc ^ *pbuf++;
        crc = table[sn] ^ (crc >> 8);
    }

    /* payload */
    pbuf = data;
    len  = datasize; 
    while (len--) {
        sn = (unsigned char)crc ^ *pbuf++;
        crc = table[sn] ^ (crc >> 8);
    }

    /* padding zeros */
    while (padsize--) {
        sn = (unsigned char)crc ^ 0;
        crc = table[sn] ^ (crc >> 8);
    }

    return crc ^= 0xFFFFFFFF;
}

int proCheckFrame(ProChan *chan, ProFrame *frame)
{
    unsigned int crc32 = 0;
    ProHead *head = PRO_HEAD(frame);
    ProTail *tail = PRO_TAIL(frame);
    char *payload = PRO_PAYLOAD(frame);
    int   paysize = PRO_PAYSIZE(frame);

    tail->magic = ntohl(tail->magic);
    tail->crc32 = ntohl(tail->crc32);

    if ( PRO_MAGIC_TAIL != tail->magic ) {
        log_i("Invalid protocol tail magic, expect[0x%8x], got[0x%08x]",
                PRO_MAGIC_TAIL, tail->magic);
        return -1;
    }

    if (head->crc)
        crc32 = proCrc32(head, payload, paysize, head->padsize);

    if ( 0 != crc32 && crc32 != tail->crc32 ) {
        log_i("CRC32 check failed! expect[0x%08X], got[0x%08X]", 
                tail->crc32, crc32);
        return -1;
    }

    head->magic     = ntohl(head->magic);
    head->version   = ntohl(head->version);
    head->frame_len = ntohl(head->frame_len);
    head->frame_cur = ntohs(head->frame_cur);
    head->frame_sum = ntohs(head->frame_sum);
    head->paylen = ntohl(head->paylen);
    head->opcode2 = ntohs(head->opcode2);

    if ( !head->frame_cur || !head->frame_sum || 
          head->frame_cur >   head->frame_sum ) {
        log_i("Invalid frame number, cur[%d], sum[%d]!", 
                head->frame_cur, head->frame_sum);
        return -1;
    }

    if (head->prod_type && chan->prod_type != head->prod_type) {
        log_i("Invalid product type[0x%x], expect[0x%x]",
                head->prod_type, chan->prod_type);
        return -1;
    }

    if (head->prod_ver && chan->prod_ver != head->prod_ver) {
        log_i("Invalid product ver[0x%x], expect[0x%x]",
                head->prod_ver, chan->prod_ver);
        return -1;
    }

    if (head->padsize && ((paysize + head->padsize) % PRO_DATA_ALIGN)) {
        log_i("Sum of payload[%d] and padsize[%d] not aligned to [%d]!", 
                paysize, head->padsize, PRO_DATA_ALIGN);
        return -1;
    }

    chan->recv_cnt++;
    log_d("Recv: cnt[%d] op1[0x%x] op2[%d] err[%d] cur[%d] sum[%d] seq[%d] plen[%d] flen[%d]",
            chan->recv_cnt, head->opcode1, head->opcode2, head->errcode,
            head->frame_cur, head->frame_sum, head->frame_seq,
            paysize, head->frame_len);
    return 0;
}

int proSendOne(ProChan *chan, void *payload, int size, int fr_cur, int fr_sum, int fr_seq,
                    int op1, int op2, int err)
{
    int ret = 0;
    unsigned int crc32 = 0;
    int paylen = round_up(size, PRO_DATA_ALIGN);
    int padsize = paylen - size;
    int framelen  = PRO_FRAME_LEN(paylen);
    ProFrame frame = {framelen, chan->sendbuff};
    ProHead *head = PRO_HEAD(&frame);
    ProTail *tail = PRO_TAIL(&frame);

    RT_ASSERT(chan);
    RT_ASSERT(chan->send);
//    RT_ASSERT(payload && size);
    RT_ASSERT(size <= PRO_MAX_PAYLEN);

    rt_memset(frame.data, 0, frame.len);
    head->magic     = htonl(PRO_MAGIC_HEAD);
    head->version   = htonl(PRO_VERSION_2);
    head->frame_len = htonl(framelen);
    head->crc       = PRO_EN_CRC32;
    head->frame_cur = htons(fr_cur);
    head->frame_sum = htons(fr_sum);
    head->paylen    = htonl(paylen);
    head->prod_type = chan->prod_type;
    head->prod_ver  = chan->prod_ver;
    head->padsize   = padsize;
    head->frame_seq = fr_seq;
    head->opcode2   = htons((unsigned short)op2);
    head->opcode1   = op1;
    head->errcode   = err;

    rt_memcmp(PRO_PAYLOAD(&frame), payload, size);

    if (head->crc)
        crc32 = proCrc32(head, (char *)payload, size, padsize);
        
    tail->crc32 = htonl(crc32);
    tail->magic = htonl(PRO_MAGIC_TAIL);
	
    ret = chan->send(chan->args, head, framelen);
    chan->send_cnt++;

    log_d("Send: cnt[%d] op1[0x%x] op2[%d] err[%d] cur[%d] sum[%d] seq[%d] plen[%d] flen[%d]",
            chan->send_cnt, op1, op2, err,
            fr_cur, fr_sum, fr_seq, size, framelen);
    return ret;
}

int proChanInit(ProChan *chan, int type, int ver, RSCallback send, void *args)
{
    RT_ASSERT(chan);
    RT_ASSERT(args);
    RT_ASSERT(send);
    RT_ASSERT(type >= 0);
    RT_ASSERT(ver >= 0);
    rt_memset(chan, 0, sizeof(*chan));

    chan->prod_type = type;
    chan->prod_ver = ver;
    chan->args = args;
    chan->send = send;

    log_i("Protocol initialized for product[0x%X], ver[%d]!", type, ver);
    return 0;
}

int proChanExit(ProChan *chan)
{
    RT_ASSERT(chan);
    rt_memset(chan, 0, sizeof(*chan));
    return 0;
}

/*操作码hash表*/
static hash_table protocol_table;

/*协议处理hash函数  op1允许范围0:OP1_MAX(200),
hash table最多存储个数HASH_MAX_SIZE(50)   */
/*---------------hash函数-----------------*/
/* |0|                  |160|       |200| */
/*                   |                    */
/*                   V                    */
/* |0|                  |30|        |50|  */
/*----------------------------------------*/
int protocol_hash_func(key_type key)
{
	if(key < 160)
		return key % 30;
	else if(key >= 160 && key <= 200)
		return (key - 160) % 20 + 30;
	else
		return 0;
}

int operation_init(void) 
{
	hash_table_init(&protocol_table, protocol_hash_func);
	return 0;
}

void register_opcode_operation(int opcode1, pro_fun fun) 
{
	if(fun == NULL) {
		log_e("the function is empty");
		return;
	}
	
	struct operation *opt = (struct operation*)rt_malloc(sizeof(struct operation));
	if(!opt) {
		log_e("register opcode1:%d failed", opcode1);
		return;
	}
	opt->fun = fun;
	/*hash插入*/
	int ret = hash_insert(&protocol_table, opcode1, opt);
	if(ret < 0)
		log_e("register opcode1[%#x] function:%p [ failed ]", opcode1, fun);
	else
		log_i("register opcode1[%#x] function:%p [ success ]", opcode1, fun);
	
}

void unregister_opcode(int opcode1)
{
	struct operation *opt;
	
	opt = hash_find(&protocol_table, opcode1);
	if(!opt)
		return;
	else{
		hash_remove(&protocol_table, opcode1);
		rt_free(opt);
	}
	log_d("unregister opcode:%d success", opt);
	return;
}

struct operation* find_operation(int opcode1)
{
	struct operation *opt;
	
	opt = hash_find(&protocol_table, opcode1);
	if(!opt)
		return NULL;
	else
		return opt;
}



