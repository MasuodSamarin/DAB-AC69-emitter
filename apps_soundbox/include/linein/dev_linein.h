#ifndef __DEV_LINEIN_H__
#define __DEV_LINEIN_H__

#include "typedef.h"

#define AUX_CHECK_ENABLE   //aux模式是否被AUX在线离线消息影响

#define AUX_IO_BIT    BIT(9)
#define AUX_DIR_SET   (JL_PORTB->DIR |= AUX_IO_BIT)
#define AUX_PU_SET    (JL_PORTB->PU  |= AUX_IO_BIT)
#define AUX_IN_CHECK  (JL_PORTB->IN  &  AUX_IO_BIT)

typedef enum __AUX_STATUS
{
    AUX_OFF = 0,
    AUX_ON,
    NULL_AUX,
}AUX_STATUS;

typedef struct __AUX_VAR
{
    u16 pre_status;
    u8  status_cnt;
    u8  bDevOnline;
    u32 last_sta;
}AUX_VAR;


void aux_init_api(void);
void aux_check_api(u32 info);
u32 aux_is_online(void);


#define PH_CHECK_ENABLE   //PH模式是否被PH在线离线消息影响

#define PH_IO_BIT    BIT(10)
#define PH_DIR_SET   (JL_PORTB->DIR |= PH_IO_BIT)
#define PH_PU_SET    (JL_PORTB->PU  |= PH_IO_BIT)
#define PH_IN_CHECK  (JL_PORTB->IN  &  PH_IO_BIT)

typedef enum __PH_STATUS
{
    PH_OFF = 0,
    PH_ON,
    NULL_PH,
}PH_STATUS;

typedef struct __PH_VAR
{
    u16 pre_status;
    u8  status_cnt;
    u8  bDevOnline;
    u32 last_sta;
}PH_VAR;


void ph_init_api(void);
void ph_check_api(u32 info);
u32 ph_is_online(void);


#endif/*__DEV_LINEIN_H__*/

