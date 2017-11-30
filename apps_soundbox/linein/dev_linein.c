#include "sdk_cfg.h"
#include "dev_linein.h"
#include "dev_manage/device_driver.h"
#include "common/msg.h"
#include "linein.h"

//ȫ�ֱ���
AUX_VAR g_aux_var; ///<line in������


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN ���߼��ʵ�庯��
   @param  cnt������˲�����
   @return �������
   @note   AUX_STATUS linein_check(u8 cnt)
*/
/*----------------------------------------------------------------------------*/
AUX_STATUS linein_check(u8 cnt)
{
    u16 cur_aux_status;

    cur_aux_status = AUX_IN_CHECK; //��ȡ��ǰAUX״̬
    if(cur_aux_status != g_aux_var.pre_status)
    {
        g_aux_var.pre_status = cur_aux_status;
        g_aux_var.status_cnt = 0;
    }
    else
    {
        g_aux_var.status_cnt++;
    }

#ifdef AUX_OUT_WITHOUT_CNT
///*//////�γ���ȥ��////////
    if((AUX_ON == g_aux_var.bDevOnline) && g_aux_var.pre_status)
    {
        g_aux_var.bDevOnline = AUX_OFF;
        return AUX_OFF;
    }
///*/////////////////////
#endif

    if(g_aux_var.status_cnt < cnt) //����
    {
        return NULL_AUX;
    }
    g_aux_var.status_cnt = 0;

    ///��⵽AUX����
    if((AUX_OFF == g_aux_var.bDevOnline) && (!g_aux_var.pre_status))
    {
        g_aux_var.bDevOnline = AUX_ON;
        return AUX_ON;
    }
#ifndef AUX_OUT_WITHOUT_CNT
    ///��⵽AUX�γ�
    else if((AUX_ON == g_aux_var.bDevOnline) && g_aux_var.pre_status)
    {
        g_aux_var.bDevOnline = AUX_OFF;
        return AUX_OFF;
    }
#endif

    return NULL_AUX;
}


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN ���߼����Ⱥ���
   @param  ��
   @return �������
   @note   s32 aux_detect(void)
*/
/*----------------------------------------------------------------------------*/
s32 aux_detect(void)
{
    u32 dev_cur_sta = DEV_HOLD;
    AUX_STATUS res;

    res = linein_check(20); //aux���߼�⣬ȥ������Ϊ50
    if(AUX_OFF == res)
    {
        aux_puts("AUX_off\n");
        dev_cur_sta = DEV_OFFLINE;
    }
    else if(AUX_ON == res)
    {
        aux_puts("AUX_on\n");
        dev_cur_sta = DEV_ONLINE;
    }

    if(g_aux_var.last_sta == dev_cur_sta)
    {
        return DEV_HOLD;//�豸״̬���ֲ���
    }

    g_aux_var.last_sta = dev_cur_sta;

    return dev_cur_sta;
}


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN ����״̬
   @param  ��
   @return LINE IN�������
   @note   u32 aux_is_online(void)
*/
/*----------------------------------------------------------------------------*/
u32 aux_is_online(void)
{
    return g_aux_var.bDevOnline;
}


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN ģ���ʼ������
   @param  ��
   @return ��
   @note   void aux_init_api(void)
*/
/*----------------------------------------------------------------------------*/
void aux_init_api(void)
{
    /*linein check port set*/
    AUX_DIR_SET;
    AUX_PU_SET;
    my_memset((u8 *)&g_aux_var, 0, sizeof(AUX_VAR));
}


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN �����б�
   @note   const u8 aux_event_tab[]
*/
/*----------------------------------------------------------------------------*/
const u8 aux_event_tab[] =
{
    SYS_EVENT_AUX_OUT, //LINE IN�γ�
    SYS_EVENT_AUX_IN,  //LINE IN����
};


/*----------------------------------------------------------------------------*/
/**@brief  LINE IN ���߼��API����
   @param  ��
   @return ��
   @note   void aux_check_api(void)
*/
/*----------------------------------------------------------------------------*/
void aux_check_api(u32 info)
{
    s32 dev_status;

    dev_status = aux_detect();
    if(dev_status != DEV_HOLD)
    {
        os_taskq_post_event(MAIN_TASK_NAME, 2, aux_event_tab[dev_status],info);
    }
}



PH_VAR g_ph_var;
PH_STATUS ph_check(u8 cnt)
{
    u16 cur_ph_status;

    cur_ph_status = PH_IN_CHECK; //��ȡ��ǰPH״̬
    if(cur_ph_status != g_ph_var.pre_status)
    {
        g_ph_var.pre_status = cur_ph_status;
        g_ph_var.status_cnt = 0;
    }
    else
    {
        g_ph_var.status_cnt++;
    }

#ifdef PH_OUT_WITHOUT_CNT
///*//////�γ���ȥ��////////
    if((PH_ON == g_ph_var.bDevOnline) && g_ph_var.pre_status)
    {
        g_ph_var.bDevOnline = PH_OFF;
        return PH_OFF;
    }
///*/////////////////////
#endif
    if(g_ph_var.status_cnt < cnt) //����
    {
        return NULL_PH;
    }
    g_ph_var.status_cnt = 0;

    ///��⵽PH����
    if((PH_OFF == g_ph_var.bDevOnline) && (!g_ph_var.pre_status))
    {
        g_ph_var.bDevOnline = PH_ON;
        return PH_ON;
    }
#ifndef PH_OUT_WITHOUT_CNT
    ///��⵽PH�γ�
    else if((PH_ON == g_ph_var.bDevOnline) && g_ph_var.pre_status)
    {
        g_ph_var.bDevOnline = PH_OFF;
        return PH_OFF;
    }
#endif

    return NULL_PH;
}

s32 ph_detect(void)
{
    u32 dev_cur_sta = DEV_HOLD;
    PH_STATUS res;

    res = ph_check(20); //ph���߼�⣬ȥ������Ϊ50
    if(PH_OFF == res)
    {
        aux_puts("PH_off\n");
        dev_cur_sta = DEV_OFFLINE;
    }
    else if(PH_ON == res)
    {
        aux_puts("PH_on\n");
        dev_cur_sta = DEV_ONLINE;
    }
    if(g_ph_var.last_sta == dev_cur_sta)
    {
        return DEV_HOLD;//�豸״̬���ֲ���
    }
    g_ph_var.last_sta = dev_cur_sta;
    return dev_cur_sta;
}

u32 ph_is_online(void)
{
    return g_ph_var.bDevOnline;
}

void ph_init_api(void)
{
    /*linein check port set*/
    PH_DIR_SET;
    PH_PU_SET;
    my_memset((u8 *)&g_ph_var, 0, sizeof(PH_VAR));
}

const u8 ph_event_tab[] =
{
    SYS_EVENT_PH_OUT, //LINE IN�γ�
    SYS_EVENT_PH_IN,  //LINE IN����
};

void ph_check_api(u32 info)
{
    s32 dev_status;

    dev_status = ph_detect();
    if(dev_status != DEV_HOLD)
    {
        os_taskq_post_event(MAIN_TASK_NAME, 2, ph_event_tab[dev_status],info);
    }
}
