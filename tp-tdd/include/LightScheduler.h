#ifndef LIGHT_SCHEDULER_H
#define LIGHT_SCHEDULER_H

#include "LightControl.h"
#include "TimeService.h"
#include "MockTimeService.h"
#include "LightControlSpy.h"

typedef struct
{
    int active; // 0 : inactive, 1 : active , this means that the light is triggered if 1
    int light_id;
    Time time;
    int state; // the action
} event_t;

enum
{
    TURN_LIGHT_OFF = 0,
    TURN_LIGHT_ON = 1
};

int LightScheduler_Create(void);
int LightScheduler_Destroy(void);
int LightScheduler_AddEvent(int, Time, int);
void LightScheduler_WakeUp(void);
void light_handler_setup(event_t *);
void light_handler_setdown(event_t *);
int LightScheduler_RemoveEvent(int, Time);
void activation_helper(event_t *, Time);
#endif
