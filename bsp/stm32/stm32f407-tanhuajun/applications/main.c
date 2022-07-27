/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */
#define LOG_TAG   "main.test"
#define LOG_LVL   LOG_LVL_DBG

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <ulog.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include "protocol.h"


#define IWDG_DEVICE_NAME "wdt"
static rt_device_t wdg_dev;

static rt_timer_t heart_timer;


#define EEPROM_I2CBUS_NAME  "i2c1"
#define EEPROM_ADDR  0x50
struct rt_i2c_bus_device *i2c_bus;

#define MAX_CLIENT_LISTEN 5

rt_list_t client_list_head;

rt_mailbox_t handle_mailbox;

//time_t ntp_sync_to_rtc(const char *host_name);
int ulog_network_backend_init(void);
void protocol_init(void);
int ntp_auto_sync_init(char* host_name);

static void idle_hook(void) {
	/*对看门狗进行喂狗*/
	rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}


#define HEARTBEAT_LED    GET_PIN(D, 10)

static void heart_fun(void *parameter) {
	static rt_int8_t refesh = 0;
	rt_tick_t time;
		switch(refesh) {
		case 0:
			time = 100;
			rt_pin_write(HEARTBEAT_LED, PIN_HIGH);
			rt_timer_control(heart_timer, RT_TIMER_CTRL_SET_TIME, &time);
			break;
		case 1:
			rt_pin_write(HEARTBEAT_LED, PIN_LOW);
			break;
		case 2:
			rt_pin_write(HEARTBEAT_LED, PIN_HIGH);
			break;
		case 3:
			time = 1000;
			rt_pin_write(HEARTBEAT_LED, PIN_LOW);
			rt_timer_control(heart_timer, RT_TIMER_CTRL_SET_TIME, &time);
			refesh = -1;
			break;
		default:
			break;
	}
	refesh++;
}


rt_err_t eeprom_read(rt_uint8_t addr, rt_uint8_t *buf, rt_uint16_t size) {
	struct rt_i2c_msg msg[2];
	
	msg[0].addr = EEPROM_ADDR;
	msg[0].flags =  RT_I2C_WR;
	msg[0].buf = &addr;
	msg[0].len = 1;
	
	msg[1].addr = EEPROM_ADDR;
	msg[1].flags = RT_I2C_RD;
	msg[1].buf = buf;
	msg[1].len = size;
	
	return rt_i2c_transfer(i2c_bus, msg, 2) == 2 ? RT_EOK : -RT_ERROR;
}

rt_err_t eeprom_write(rt_uint8_t addr, rt_uint8_t *buf, rt_uint16_t size) {
	struct rt_i2c_msg msg[2];
	
	msg[0].addr = EEPROM_ADDR;
	msg[0].flags =  RT_I2C_WR;
	msg[0].buf = &addr;
	msg[0].len = 1;
	
	msg[1].addr = EEPROM_ADDR;
	msg[1].flags = RT_I2C_WR | RT_I2C_NO_START;
	msg[1].buf = buf;
	msg[1].len = size;
	
	return rt_i2c_transfer(i2c_bus, msg, 2) == 2 ? RT_EOK : -RT_ERROR;
}

int main(void)
{
	rt_err_t ret = RT_EOK;
	rt_uint32_t wdg_timeout = 5; /*s*/
	rt_uint16_t port;

    /*初始化心跳指示灯引脚*/
    rt_pin_mode(HEARTBEAT_LED, PIN_MODE_OUTPUT);
	
	heart_timer = rt_timer_create("heart",
						          heart_fun, 
	                              RT_NULL,
	                              1000,
	                              RT_TIMER_FLAG_PERIODIC | 
	                              RT_TIMER_FLAG_SOFT_TIMER);
	if(heart_timer != RT_NULL)
		/*启动定时器*/
		rt_timer_start(heart_timer);
	
	LOG_E("this is a test");
	LOG_W("this is a test");
	LOG_I("this is a test");
	LOG_D("this is a test");
	
	/*查找i2c设备*/
	i2c_bus = (struct rt_i2c_bus_device*)rt_device_find(EEPROM_I2CBUS_NAME);
	if(i2c_bus == RT_NULL) {
		rt_kprintf("can't find %s device!\n", EEPROM_I2CBUS_NAME);
		return RT_ERROR;
	}
	
//	port = ntohs(8025);
//	ret = eeprom_write(0x0e, (rt_uint8_t*)&port, 2);
//	if(ret == RT_EOK)
//		rt_kprintf("eeprom write success\n");
//	
//	ret = eeprom_read(0x0e, (rt_uint8_t*)&port, 2);
//	if(ret == RT_EOK)
//		rt_kprintf("%d\n", htons(port));
	
	/*查找设备*/
	wdg_dev = rt_device_find(IWDG_DEVICE_NAME);
	if(!wdg_dev) {
		rt_kprintf("find %s failed!\n", IWDG_DEVICE_NAME);
		return RT_ERROR;
	}
	
	/*初始化设备*/
	ret = rt_device_init(wdg_dev);
	if(ret != RT_EOK) {
		rt_kprintf("initialize %s failed!\n", IWDG_DEVICE_NAME);
		return ret;
	}
	
	/*设置看门狗溢出时间*/
	ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &wdg_timeout);
	if(ret != RT_EOK) {
		rt_kprintf("set %s timeout failed!\n", IWDG_DEVICE_NAME);
		return ret;
	}
	
	/*设置空闲线程钩子函数*/
	rt_thread_idle_sethook(idle_hook);
	
	/*启动看门狗*/
	ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, NULL);
	if(ret != RT_EOK) {
		rt_kprintf("start wdg failed!\n");
		return ret;
	}
	
//	ntp_sync_to_rtc("192.168.100.77");
	protocol_init();

	return RT_EOK;
}



int tcp_channel_send(void *arg, void *buf, int len) {
	int fd = (int)arg;
	return send(fd, buf, len, 0);
}


static void server_thread_entry(void *param) {
	int listen_fd;
	int ret;
	struct sockaddr_in server_addr;
	int client_fds[MAX_CLIENT_LISTEN];
	
	for(int i=0; i<MAX_CLIENT_LISTEN; i++)
		client_fds[i] = -1;
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0) {
		rt_kprintf("create socket failed\n");
		return;
	}
	
	rt_memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8025);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret != 0) {
		rt_kprintf("server bind failed\n");
		closesocket(listen_fd);
		return;
	}
	
	ret = listen(listen_fd, MAX_CLIENT_LISTEN);
	if(ret != 0) {
		rt_kprintf("server listen failed\n");
		closesocket(listen_fd);
		return;		
	}
	
	int max_fd;
	int client_fd;
	struct sockaddr_in client_addr;
	char client_ip[IP4ADDR_STRLEN_MAX];
	socklen_t client_addr_len = sizeof(client_addr);
	fd_set read_set;
	
	int keep_alive = 1;
	int keep_idle = 5;
	int keep_interval = 5;
	int keep_count = 2;
	int flag = 1;
	
	char ntp_init = 1;
	
	char mutex_name[20];
	
	while(1) {
		
		max_fd = -1;
		FD_ZERO(&read_set);
		for(int i=0; i<MAX_CLIENT_LISTEN; i++) {
			if(client_fds[i] != -1) {
				FD_SET(client_fds[i], &read_set);
				if(max_fd < client_fds[i])
					max_fd = client_fds[i];
			}
		}
		
		FD_SET(listen_fd, &read_set);
		max_fd = max_fd > listen_fd ? max_fd : listen_fd;

		ret = select(max_fd + 1, &read_set, RT_NULL, RT_NULL, RT_NULL);		
		/*0:超时 <0:出错*/
		if(ret <= 0) {
			rt_kprintf("select error\n");
			continue;
		}

		if(FD_ISSET(listen_fd, &read_set)) {
			
			client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
			if(client_fd < 0) {
				rt_kprintf("server accept error\n");
				continue;
			}
			
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, IP4ADDR_STRLEN_MAX);
			rt_kprintf("new client connect, fd:%d, ip:%s, port:%d\n", client_fd, client_ip, ntohs(client_addr.sin_port));

			ntp_init = ntp_init ? ntp_auto_sync_init(client_ip) : 0;
			
			/*申请chan*/
			ProChan *client_chan = rt_malloc(sizeof(ProChan));
			if(client_chan == RT_NULL) {
				rt_kprintf("malloc channal failed\n");
				closesocket(client_fd);
				continue;
			}
			
			/*初始化channal*/
			proChanInit(client_chan, 0x18, 0x01, tcp_channel_send, (void*)client_fd);
			
			/*初始化接收buffer互斥体*/
			rt_snprintf(mutex_name, sizeof(mutex_name), "read_fd%d", client_fd);
			client_chan->recv_mutex = rt_mutex_create(mutex_name, RT_IPC_FLAG_FIFO);
			if(client_chan->recv_mutex == RT_NULL) {
				rt_kprintf("socket[%d] mutex init failed\n", client_fd);
				rt_free(client_chan);
				closesocket(client_fd);
				continue;	
			}

			/*将channal加入客户端链表*/
			rt_list_insert_after(&client_list_head, &client_chan->list);
			
			/*keepalive*/
			setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keep_alive, sizeof(keep_alive));
			setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keep_idle, sizeof(keep_idle));
			setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keep_interval, sizeof(keep_interval));
			setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keep_count, sizeof(keep_count));
			setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(flag));		
			
			/*发送心跳帧*/
			proSendNull(client_chan, OP1_HRT_U2L, 0, 0);
				
			for(int i=0; i<MAX_CLIENT_LISTEN; i++) {
				if(client_fds[i] == -1) {
					client_fds[i] = client_fd;
					break;
				}
			}
		}
		
		rt_list_t *pos;
		ProChan *chan;
		for(int i=0; i<MAX_CLIENT_LISTEN; i++) {
			if(client_fds[i] != -1 && FD_ISSET(client_fds[i], &read_set)) {
				
				rt_list_for_each(pos, &client_list_head) {
					chan = rt_list_entry(pos, ProChan, list);
					if((int)chan->args == client_fds[i]) {
						
						/*获取接收buffer互斥体  处理线程在处理这个buffer数据的时候，接收线程有接收数据*/
						rt_mutex_take(chan->recv_mutex, RT_WAITING_FOREVER);
						ret = recv(client_fds[i], chan->recvbuff, PRO_MAX_SIZE, 0);
						rt_mutex_release(chan->recv_mutex);
						
						if(ret > 0) {
							chan->recv_size = ret;
							/*将该通道通过邮箱发送给协议处理线程*/
							/*非阻塞发送方式，如果邮箱已满直接返回，该帧丢弃*/
							rt_mb_send(handle_mailbox, (rt_ubase_t)chan);
						}
						else {
							log_i("client close, fd:%d", client_fds[i]);
							/*关闭socket描述符*/
							closesocket(client_fds[i]);
							client_fds[i] = -1;		
							/*删除互斥体*/
							rt_mutex_delete(chan->recv_mutex);
							/*释放chan*/
							rt_free(chan);
						}
					}
				}
			}
		}
	}
}


int proCmdExec(ProChan *chan, ProFrame *frame) {
	struct operation *opt;
	snd_msg snd;
	rev_msg rev;
	pro_ret ret;
	char send_data_buf[PRO_MAX_PAYLEN];
	
	ProHead *head = PRO_HEAD(frame);
	
	int op1 = head->opcode1;
	int op2 = head->opcode2;
	
	char *payload = PRO_PAYLOAD(frame);
	int paysize = PRO_PAYSIZE(frame);
	
	opt = find_operation(op1);
	if(!opt) {
		log_e("not found opcode1 0x%x\n", op1);
		return -1;
	}

	rev.opcode2 = op2;
	rev.buf = payload;
	rev.size = paysize;
	rev.frame_cur = head->frame_cur;
	rev.frame_sum = head->frame_sum;
	
	snd.size = 0;
	snd.op2 = head->opcode2;
	snd.op1 = head->opcode1;

	rt_memset(send_data_buf, 0, PRO_MAX_PAYLEN);
	snd.buf = send_data_buf;
	/*执行操作回调函数*/
	ret = opt->fun(&snd, &rev);
	/*重新赋值op*/
	head->opcode2 = snd.op2;
	head->opcode1 = snd.op1;
	/*协议帧只返回应答*/
	if(!snd.size)
		proSendAckNull(chan, head, ret);
	else if(snd.size > PRO_MAX_PAYLEN) {
		rt_kprintf("the data size is out %d\n", PRO_MAX_PAYLEN);
		return -1;
	} else {
		/*协议帧返回数据*/
		proSendAckData(chan, head, snd.buf, snd.size, ret);
	}
	return 0;
}

int proParseFrame(ProChan *chan, void *buff, int len)
{
    int ret = 0;
    ProFrame frame = {0};
	
    ret = proFindHead(chan, buff, len, &frame);
    if (ret < 0)
        return ret;
    ret = proCheckFrame(chan, &frame);
    if (ret < 0)
        return ret;
    return proCmdExec(chan, &frame);
}


static void handle_thread_entry(void *param) {
	ProChan *chan;
	while(1) {
		/*阻塞读取mailbox信息*/
		rt_mb_recv(handle_mailbox, (rt_ubase_t*)&chan, RT_WAITING_FOREVER);
		
		/*获取read_buf锁*/
		rt_mutex_take(chan->recv_mutex, RT_WAITING_FOREVER);

		proParseFrame(chan, chan->recvbuff, chan->recv_size);

		/*释放read_buf锁*/
		rt_mutex_release(chan->recv_mutex);
	}
}


/*回复心跳*/
pro_ret send_heart(snd_msg *snd, rev_msg *rev) {
	return SUCCEED; 
}


void protocol_init(void) {

	/*初始化客户端链表*/
	rt_list_init(&client_list_head);
	
	/*初始化邮箱*/
	handle_mailbox = rt_mb_create("hand_mb", 10, RT_IPC_FLAG_FIFO);
	if(handle_mailbox == RT_NULL)
		rt_kprintf("handle mailbox creat failed\n");
	
	/*初始化协议相关*/
	operation_init();
	
	rt_thread_t server_tid = rt_thread_create("server",
							server_thread_entry,
							RT_NULL,
							1300,
							4,
							20);
	if(server_tid != RT_NULL)
		rt_thread_startup(server_tid);
	else
		rt_kprintf("server thread failed\n");
	
	
	rt_thread_t handle_tid = rt_thread_create("handle",
												handle_thread_entry,
												RT_NULL,
												1500,
												3,
												100);
	if(handle_tid != RT_NULL)
		rt_thread_startup(handle_tid);
	else
		rt_kprintf("handle thread failed\n");
	
	register_opcode_operation(OP1_HRT_U2L, send_heart);
}


#if 0
void server_handle_entry(void *param) {
	int fd = (int)param;
	int ret;
	char buf[20];
	while(1) {
		ret = recv(fd, buf, 20, 0);
		buf[19] = '\0';
		if(ret > 0) {
			log_i("read date: %s", buf);
		}
		else if(ret <= 0) { /*断开连接0  存活机制网络拔出-1*/
			rt_kprintf("error return:%d", ret);
			break;
		}
	}
	log_i("client close, fd:%d\n", fd);
	closesocket(fd);
}

void protocol_init(void) {
	int ser_fd;
	int ret;
	struct sockaddr_in server_addr;
	
	ser_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(ser_fd < 0) {
		rt_kprintf("server error\n");
		return;
	}

	rt_memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8025);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	ret = bind(ser_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret != 0) {
		rt_kprintf("server bind error\n");
		closesocket(ser_fd);
		return;
	}
	
	ret = listen(ser_fd, 5);
	if(ret != 0) {
		rt_kprintf("server listen error\n");
		closesocket(ser_fd);
		return;
	}
	
	struct sockaddr_in client_addr;
	int cli_fd;
	socklen_t len = sizeof(client_addr);
	char cli_ip[IP4ADDR_STRLEN_MAX];
	
	int keep_alive = 1;
	int keep_idle = 30;
	int keep_interval = 10;
	int keep_count = 2;
	int flag = 1;
		
	char client_name[10];
	int client_num = 0;
	static char ntp_init = 1;
	
	while(1) {
		
		cli_fd = accept(ser_fd, (struct sockaddr*)&client_addr, &len);
		if(cli_fd < 0) {
			rt_kprintf("server accept error\n");
			continue;
		}
		
		inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, IP4ADDR_STRLEN_MAX);
		rt_kprintf("new client connect, fd:%d, ip:%s, port:%d\n", cli_fd, cli_ip, ntohs(client_addr.sin_port));
		
		/*设置ntp服务器ip*/
		ntp_init = ntp_init ? ntp_auto_sync_init(cli_ip) : 0;
		
		if(setsockopt(cli_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keep_alive, sizeof(keep_alive)) < 0) {
			rt_kprintf("set keep_alive failed\n");
			return;
		}
		
		/*set keepalive*/
		setsockopt(cli_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keep_idle, sizeof(keep_idle));
		setsockopt(cli_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keep_interval, sizeof(keep_interval));
		setsockopt(cli_fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keep_count, sizeof(keep_count));
		setsockopt(cli_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(flag));
		
		rt_snprintf(client_name, sizeof(client_name), "cli_t%d", client_num++);
		
		rt_thread_t tid = rt_thread_create(client_name,
											server_handle_entry, 
											(void*)cli_fd,
											1024,
											3,
											20);
		if(tid != RT_NULL)
			rt_thread_startup(tid);
	}
}

#endif
