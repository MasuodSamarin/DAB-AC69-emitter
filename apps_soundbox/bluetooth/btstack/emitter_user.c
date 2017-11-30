#include "bluetooth/avctp_user.h"
#include "aec/aec_api.h"
#include "common/app_cfg.h"
#include "dac/dac_api.h"
#include "common/msg.h"
#include "sdk_cfg.h"
#include "clock.h"
#include "rtc/rtc_api.h"
#include "play_sel/play_sel.h"
#include "uart_param.h"
#include "emitter_user.h"
#include "dac/ladc.h"
#include "dev_linein.h"
#include "rtos/task_manage.h"

u8 MCU_mute_flag = 0;

EMITTER_INFO_T emitter_info;

void set_emitter_role(u8 role)
{
	emitter_info.role = role;
	__set_emitter_role(role);
}

u8 get_emitter_role(void)
{
	return emitter_info.role;
}

void set_emitter_aux_media(u8 aux_en)
{
	if(aux_en) {
		if(emitter_info.a2dp_source_media == A2DP_SOURCE_AUX)
			return;
		emitter_info.a2dp_source_media = A2DP_SOURCE_AUX;
		emitter_aux_open();
//		os_time_dly(35);
		__emitter_send_media_toggle(aux_en);
	} else {
		if(emitter_info.a2dp_source_media == A2DP_SOURCE_AUX)
		{
			emitter_info.a2dp_source_media = A2DP_SOURCE_NULL;
			__emitter_send_media_toggle(aux_en);
			emitter_aux_close();
		}
	}
}

void set_emitter_decode_media(u8 dec_en)
{
	if(dec_en) {
		if(emitter_info.a2dp_source_media == A2DP_SOURCE_DEC)
			return;
		emitter_info.a2dp_source_media = A2DP_SOURCE_DEC;
		__emitter_send_media_toggle(dec_en);
	} else {
		emitter_info.a2dp_source_media = A2DP_SOURCE_NULL;
		__emitter_send_media_toggle(dec_en);
	}
}

u8 src_task_priority()
{
	return TaskSrcPrio;
}

//#define SEARCH_BD_ADDR_LIMIITED
u8 bd_addr_filt[][6] =
{
	{0x8E,0xA7,0xCA,0x0A,0x5E,0xC8},/*S10_H*/
	{0xA7,0xDD,0x05,0xDD,0x1F,0x00},/*ST-001*/
	{0xE9,0x73,0x13,0xC0,0x1F,0x00},/*HBS 730*/
	{0x38,0x7C,0x78,0x1C,0xFC,0x02},/*Bluetooth*/
};
u8 search_bd_addr_filt(u8 *addr)
{
	u8 i;
	puts("bd_addr:");
	put_buf(addr,6);
	for(i=0;i<(sizeof(bd_addr_filt)/sizeof(bd_addr_filt[0]));i++)
	{
		if(memcmp(addr,bd_addr_filt[i],6) == 0) {
			printf("bd_addr match:%d\n",i);
			return 0;
		}
	}
	puts("bd_addr not match\n");
	return 1;
}

//#define SEARCH_BD_NAME_LIMIITED
#if 0
u8 bd_name_filt[][32] =
{
	"BT-SPEAKER",
	"CSC608"
	"QCY-QY19",
	"Newmine",
	"HT1+",
	"S-35",
	"HUAWEI AM08",
	"AC69_Bluetooth",
	"BV200",
	"MNS",
	"Jam Heavy Metal",
	"Bluedio",
	"HR-686",
	"BT MUSIC",
	"BW-USB-DONGLE",
	"S530",
	"XPDQ7",
	"MICGEEK Q9S",
	"S10_H",
	"HBS-730",
	"SPORT-S9",
	"Q5",
	"IAEB25",
	"QY7",
	"MS-808",
	"LG HBS-730",
	"NG-BT07"
};
#else
u8 bd_name_filt[][32] =
{
//	"AC69_BT_SDK",
	"BIN",
	"XIN",
	"K9"
};
#endif

u8 search_bd_name_filt(u8 *data,u8 len)
{
	char bd_name[32] = {0};
	u8 i,bd_name_len;

	if(len >(sizeof(bd_name))) {
		printf("bd_name_len error:%d\n",len);
		return 1;
	}

	memcpy(bd_name,data,len);
	bd_name_len = len;
	printf("search_bd_name:%s,len:%d\n",bd_name,len);
    user_profile_send(USER_CMD_GET_BT_NAME_SEARCH,(u8 *)bd_name,len);

	for(i=0;i<(sizeof(bd_name_filt)/sizeof(bd_name_filt[0]));i++)
	{
		if(memcmp(data,bd_name_filt[i],bd_name_len) == 0) {
			puts("\n*****find dev ok******\n");
            user_profile_send(USER_CMD_GET_BT_NAME,(u8 *)data,bd_name_len);
			return 0;
		}
	}

	return 1;
}
struct __search_bd_result {
    char bd_name[32];
    u8   bd_addr[6];
};
static struct __search_bd_result search_bd_result[10] = {0,0};
static u8 *target_connect = NULL;
u8 save_search_bd_result(char *bd_name,u8 name_len,u8 *addr)
{
    u8 idx = 0;
    for(idx = 0; idx < 10; idx++)
    {
        if(memcmp(search_bd_result[idx].bd_addr,addr,6) == 0){
            puts("alreadly save\n");
            return 0;
        }
        if(search_bd_result[idx].bd_addr[0] == 0x00){
            break;
        }
    }
    if(idx == 10) {
        puts("no free space\n");
        return 0;
    }
    memcpy(search_bd_result[idx].bd_name, bd_name, name_len);
    memcpy(search_bd_result[idx].bd_addr, addr, 6);
    printf("search_bd_result:%d, name:%s, addr:",idx,search_bd_result[idx].bd_name);
    put_buf(search_bd_result[idx].bd_addr,6);

    user_profile_send(USER_CMD_GET_BT_NAME_SEARCH,(u8 *)search_bd_result[idx].bd_name,name_len);
//	user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR,6,addr);
    return 1;
}
u8 * match_bt_name(char *bd_name,u8 name_len)
{
    u8 idx = 0;
	for(idx = 0;idx < 10; idx++)
	{
		if(memcmp(bd_name,search_bd_result[idx].bd_name,name_len) == 0) {
			puts("\n*****find dev ok******\n");
			target_connect = search_bd_result[idx].bd_addr;
            put_buf(search_bd_result[idx].bd_addr,6);
            return search_bd_result[idx].bd_addr;
		}
	}
    puts("\n*****no find dev******\n");
    return search_bd_result[0].bd_addr;
}
u8 * get_connect_target(void)
{
    return target_connect;
}
void delete_search_bd_result(void)
{
    target_connect = NULL;
	memset(search_bd_result, 0x00, sizeof(struct __search_bd_result)*10);
}
#define BD_INQUIRY_RESULT_CUSTOM
#ifdef BD_INQUIRY_RESULT_CUSTOM
u8 bd_inquiry_result_custom(char *bd_name,u8 name_len,u8 *addr)
{
	char bt_name[32] = {0};
	u8 len = 0;

	if(name_len == 0) {
		puts("No_eir\n");
		char *unknown_name = "unknown";
		len = strlen(unknown_name);
        memcpy(bt_name,unknown_name,len);
	}
	else {
		len	= name_len;
		if(len > 32)
        {
        #if 0
			len = 31;
            /* display bd_name */
            memcpy(bt_name,bd_name,len);
        #else
            char *overlong_name = "overlong";
            len = strlen(overlong_name);
            memcpy(bt_name,overlong_name,len);
        #endif // 0
        }else{
            memset(bt_name, 0x00, 32);
            /* display bd_name */
            memcpy(bt_name,bd_name,len);
        }
		printf("inquiry_bd_name:%s,len:%d\n",bt_name,len);
	}
	/* display bd_addr */
	put_buf(addr,6);

	save_search_bd_result(bt_name,len,addr);
    /* You can connect the specified bd_addr by below api      */
//	user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR,6,addr);

	return 0;
}
#endif

/*
	Inquiry again will ignore the first few bd_addr according to SEARCH_BD_ADDR_FILT.
    Define SEARCH_BD_ADDR_FILT as 0 means all bd_addrs are valid by inquiried.
*/
#define SEARCH_BD_ADDR_FILT     0 ///<maximum:10

void emitter_init(u8 role)
{
	//emitter role shoule set firstly
	set_emitter_role(role);

#ifdef SEARCH_BD_ADDR_LIMIITED
	search_bd_addr_filt_handle_register(search_bd_addr_filt);
#endif

#ifdef SEARCH_BD_NAME_LIMIITED
	search_bd_name_filt_handle_register(search_bd_name_filt);
#endif

#ifdef BD_INQUIRY_RESULT_CUSTOM
	bd_inquiry_result_custom_register(bd_inquiry_result_custom);
	__set_inquiry_again_flag(0);//0:inquiry complete	1:inquiry again when inquiry complete,unless dev match
#else
	__set_inquiry_again_flag(0);//0:inquiry complete	1:inquiry again when inquiry complete,unless dev match
#endif

	__set_bd_addr_filt(SEARCH_BD_ADDR_FILT);

	memset(search_bd_result, 0x00, sizeof(struct __search_bd_result)*10);
}

#define USER_CMD_START  0xAA
#define USER_CMD_END    0xA5

u8 disconnect_bt_name[32] = "No device";
u8 remote_bt_name[32] = "No device";
static u8 connect_flags = 0;
void save_bt_conect_flag(u8 flag)
{
    connect_flags = flag;
}
void get_remote_bt_name(u8 *data)
{
    if(strlen((char *)data) < 32)
    {
        printf("get_remote_bt_name:%s\n",data);
        sprintf((char *)remote_bt_name,"%s",(char *)data);
        printf("get_remote_bt_name:%s\n",remote_bt_name);
    }
}
void reset_remote_bt_name(u8 *data)
{
    sprintf((char *)remote_bt_name,"%s",(char *)disconnect_bt_name);
    printf("reset_remote_bt_name:%s\n",remote_bt_name);
}
u8 *judge_bt_name(void)
{
    if((BT_STATUS_WAITINT_CONN == get_bt_connect_status())||(connect_flags == 0)){
        printf("judge_bt_name:%s\n",disconnect_bt_name);
        return disconnect_bt_name;
    }else{
        printf("judge_bt_name:%s\n",remote_bt_name);
        return remote_bt_name;
    }
}
extern u8 user_profile_parse(u8 *data,u8 len);
extern void user_uart_write(char a);
static u8 user_cmd_data_buf[128];
static u8 writepos = 0;
void user_uart_isr_callback(u8 uto_buf,void *p, u8 isr_flag)
{
    u8 * ptr;
//    putchar('U');
    if( UART_ISR_TYPE_DATA_COME == isr_flag)
    {
    putchar('S');
    put_u8hex(uto_buf);
        ptr = (u8 *)&user_cmd_data_buf;
        ptr[writepos] = uto_buf;
        if(uto_buf == USER_CMD_START)
        {
            writepos = 0;
            ptr[writepos] = uto_buf;
            ptr[1] = 0;
            ptr[2] = 0;
        }
        writepos++;
//        if(ptr[writepos] == USER_CMD_END)
        if((writepos > 2)&&(ptr[2]+3 == writepos))
        {
            user_profile_parse(ptr,writepos);
            writepos = 0;
        }
//        user_uart_write(uto_buf);
    }
}

void user_uart_write_repeat(u8 *data,u8 len)
{
    u8 tmp;
    if((len == 0)||(data == NULL)) return;
    for(tmp = 0;tmp < len; tmp++)
    {
        user_uart_write(*(data+tmp));
    }
}
u8 user_profile_parse(u8 *data,u8 len)
{
    ENUM_USER_CMD_TYPE cmd;
    u8 *pdata = data;
    if(USER_CMD_START != pdata[0]) return 0;
    if(USER_CMD_MAX > pdata[1])
        cmd = pdata[1];
    else
        return 0;
    switch(cmd)
    {
        case USER_CMD_GET_BT_NAME:
            puts("\nUSER_CMD_GET_BT_NAME:");
            put_buf(pdata+3,len-3);
            break;
        case USER_CMD_GET_AUX_STATUS:
            puts("\nUSER_CMD_GET_AUX_STATUS:");
            put_buf(pdata+3,len-3);
            break;
        case USER_CMD_GET_BT_SWITCH:
            puts("\nUSER_CMD_GET_AUX_STATUS:");
            put_buf(pdata+3,len-3);
            os_taskq_post_msg(MAIN_TASK_NAME, 1, MSG_SWITCH_EMITTER_ROLE);
            break;
        case USER_CMD_GET_INPUT_SWITCH:
            puts("\nUSER_CMD_GET_INPUT_SWITCH:");
            put_buf(pdata+3,len-3);
            os_taskq_post(MAIN_TASK_NAME,3,MSG_USER_CMD,USER_CMD_GET_INPUT_SWITCH,pdata[3]);
            break;
        case USER_CMD_CONNECTED:
            puts("\nUSER_CMD_CONNECTED:");
            put_buf(pdata+3,len-3);
            match_bt_name((char *)(pdata+3),len-3);
//            os_taskq_post_msg("btmsg", 1, MSG_BT_STOP_INQUIRY);
//            os_time_dly(5);//中断函数，不能延时
            if((get_connect_target() != NULL)){
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR,6,get_connect_target());
            }
            break;
        case USER_CMD_DISCONNECT_AND_RESCAN:
            puts("\nUSER_CMD_DISCONNECT_AND_RESCAN:");
            put_buf(pdata+3,len-3);
            delete_search_bd_result();
            os_taskq_post_msg("btmsg", 1, MSG_BT_DELETE_REMOTE_INFO);
            os_taskq_post_msg("btmsg", 1, MSG_BT_CONNECT_CTL);
            if(get_emitter_role() == BD_ROLE_HOST)
            {
                user_send_cmd_prepare(USER_CTRL_SEARCH_DEVICE,0,NULL);
            }
            break;
         case USER_CMD_AUDIO_MUTE:
            puts("\nUSER_CMD_AUDIO_MUTE:");
            put_buf(pdata+3,len-3);
            if(pdata[3] == 0){
//                set_emitter_aux_media(1);
//                os_taskq_post(MAIN_TASK_NAME,3,MSG_USER_CMD,USER_CMD_AUDIO_MUTE,pdata[3]);
            }
            else{
                os_taskq_post(MAIN_TASK_NAME,3,MSG_USER_CMD,USER_CMD_AUDIO_MUTE,pdata[3]);
//                set_sys_vol(0,0,FADE_ON);
//                emitter_aux_close();
//                if(BT_STATUS_WAITINT_CONN != get_bt_connect_status()){
//                    set_emitter_aux_media(1);
//                    set_emitter_aux_media(0);
//                }
            }
//            set_emitter_aux_media(1);
            break;
         case USER_CMD_BT_NEXT_FILE:
            puts("\nUSER_CMD_BT_NEXT_FILE:");
            put_buf(pdata+3,len-3);
            os_taskq_post_msg("btmsg", 1, MSG_BT_NEXT_FILE);
            break;
         case USER_CMD_BT_PREV_FILE:
            puts("\nUSER_CMD_BT_PREV_FILE:");
            put_buf(pdata+3,len-3);
            os_taskq_post_msg("btmsg", 1, MSG_BT_PREV_FILE);
            break;
         case USER_CMD_BT_PP:
            puts("\nUSER_CMD_BT_PP:");
            put_buf(pdata+3,len-3);
            if(compare_task_name("BtStackTask"))
                os_taskq_post_msg("btmsg", 1, MSG_BT_PP);
            else
                os_taskq_post_msg(LINEIN_TASK_NAME, 1, MSG_AUX_MUTE);
            break;
         case USER_CMD_POWER:
            puts("\nUSER_CMD_POWER:");
            put_buf(pdata+3,len-3);
            break;
         case USER_CMD_CLEAR_PLOSIVE:
            puts("\nUSER_CMD_CLEAR_PLOSIVE:");
            put_buf(pdata+3,len-3);
            os_taskq_post(MAIN_TASK_NAME,3,MSG_USER_CMD,USER_CMD_CLEAR_PLOSIVE,pdata[3]);
            break;
       default:
            break;
    }
    return 0;
}
u8 emulate_uart_receive(void)
{
    static u16 timer_cnt = 0;
    u8 data[] = {0xAA, 0x07, 0x01 };
    if((BT_STATUS_WAITINT_CONN != get_bt_connect_status()))
    {
        timer_cnt++;
    }else{
        timer_cnt = 0;
    }
    if(timer_cnt > 100)
    {
        puts("emulate_uart_receive\n");
        user_profile_parse( data, 3 );
    }
    return 0;
}
u8 user_profile_send(ENUM_USER_CMD_TYPE cmd,u8 *data,u8 len)
{
    u8 *pdata = data;
    user_uart_write(USER_CMD_START);
    switch(cmd)
    {
        case USER_CMD_GET_BT_NAME_SEARCH:
            user_uart_write(USER_CMD_GET_BT_NAME_SEARCH);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_GET_BT_NAME:
            user_uart_write(USER_CMD_GET_BT_NAME);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_GET_AUX_STATUS:
            user_uart_write(USER_CMD_GET_AUX_STATUS);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_GET_PH_STATUS:
            user_uart_write(USER_CMD_GET_PH_STATUS);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_GET_BT_SWITCH:
            user_uart_write(USER_CMD_GET_BT_SWITCH);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_GET_INPUT_SWITCH:
            user_uart_write(USER_CMD_GET_INPUT_SWITCH);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_CONNECTED:
            user_uart_write(USER_CMD_CONNECTED);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        case USER_CMD_DISCONNECT_AND_RESCAN:
            user_uart_write(USER_CMD_DISCONNECT_AND_RESCAN);
            user_uart_write(len);
            user_uart_write_repeat(pdata,len);
            break;
        default:
            break;
    }
//    user_uart_write(USER_CMD_END);
    return 0;
}

void user_profile_test(void)
{
    static u8 circle_idx = 0;
    char *str = "XIN";
    u8 data;

    circle_idx++;
    if( circle_idx > 10 )    circle_idx = 0;
    switch(circle_idx)
    {
        case 0:
        {
            u8 *bt_name_ptr = judge_bt_name();
            user_profile_send(USER_CMD_GET_BT_NAME,(u8 *)(bt_name_ptr),strlen((char *)(bt_name_ptr)));///
        }
        break;
        case 1:
//            user_profile_send(USER_CMD_GET_BT_NAME_SEARCH,(u8 *)str,strlen(str));
            data = aux_is_online();
            user_profile_send(USER_CMD_GET_AUX_STATUS,&data,1);
        break;
        case 2:
            data = ph_is_online();
            user_profile_send(USER_CMD_GET_PH_STATUS,&data,1);
        break;
        case 3:
            data = get_emitter_role();
            user_profile_send(USER_CMD_GET_BT_SWITCH,&data,1);
        break;
        case 4:
//            data = 2;
//            user_profile_send(USER_CMD_GET_INPUT_SWITCH,&data,1);
        break;
        case 5:
        break;
        case 6:
        break;
    }
}
