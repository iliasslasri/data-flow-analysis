#include "LightScheduler.h"


static int NUMBER_OF_SCHEDULES = 0;

event_t events[256];

static int handler = 0;

int LightScheduler_Create(void)
{
    LightControl_init();
    TimeService_init();
    
    NUMBER_OF_SCHEDULES = 0;

    for (int i = 0; i < 256; i++){
        events[i].active = 0;
        events[i].light_id = 0;
        events[i].time.dayOfWeek = 0;
        events[i].time.minuteOfDay = 0;
        events[i].state = 0;
    }

    // should return the handler to be able to stop the alarm
    TimeService_startPeriodicAlarm(60, &LightScheduler_WakeUp);
    return 1;
}

int LightScheduler_Destroy(void)
{
    TimeService_stopPeriodicAlarm(handler);
    TimeService_destroy();

    LightControl_destroy();

    NUMBER_OF_SCHEDULES = 0;
    return 1;
}

/// @brief add an event to the schedule
/// @return the number of events in the schedule if the event is added successfully, -1 otherwise
int LightScheduler_AddEvent(int id, Time time, int state_trigger)
{
    if(NUMBER_OF_SCHEDULES == 256){
        return -1;
    }
    if( id < 0 || id > 255){
        return -1;
    }
    if( state_trigger != TURN_LIGHT_ON && state_trigger != TURN_LIGHT_OFF){
        return -1;
    }

    if(time.minuteOfDay < 0 || time.minuteOfDay >= 1440){
        return -1;
    }

    if(time.dayOfWeek < 1 || (time.dayOfWeek > 7 && time.dayOfWeek != 10 && time.dayOfWeek != 11 && time.dayOfWeek != 12)){
        return -1;
    }
    // check the event is not already in the schedule
    for (int i = 0; i < NUMBER_OF_SCHEDULES; i++){
        if(events[i].light_id == id && events[i].time.dayOfWeek == time.dayOfWeek && events[i].time.minuteOfDay == time.minuteOfDay){
            events[i].state = state_trigger;
            return NUMBER_OF_SCHEDULES;
        }
    }

    events[NUMBER_OF_SCHEDULES].active = 0;
    events[NUMBER_OF_SCHEDULES].light_id = id;
    events[NUMBER_OF_SCHEDULES].time = time;
    events[NUMBER_OF_SCHEDULES].state = state_trigger;

    return ++NUMBER_OF_SCHEDULES;
}

/// @brief Remove an event from the schedule
/// @return Number of events in the schedule if the event is removed successfully, -1 otherwise
int LightScheduler_RemoveEvent(int id, Time time)
{
    for (int i = 0; i < NUMBER_OF_SCHEDULES; i++){
        if(events[i].light_id == id && events[i].time.dayOfWeek == time.dayOfWeek && events[i].time.minuteOfDay == time.minuteOfDay){
            for (int j = i; j < NUMBER_OF_SCHEDULES - 1; j++){
                events[j] = events[j + 1];
            }
            NUMBER_OF_SCHEDULES--;
            return NUMBER_OF_SCHEDULES;
        }
    }
    return -1;
}

/// @brief Helper function to handle the activation of the event, if the event is active, it will be set down, otherwise it will be set up
void activation_helper(event_t *event, Time time){
    if(event->time.minuteOfDay == time.minuteOfDay){
        if (event->state == TURN_LIGHT_ON){
            LightControl_on(event->light_id);
        } else { LightControl_off(event->light_id); }
        event->active = 1;
    }
}

/// @brief Wake up the light scheduler, check the time and trigger the events
void LightScheduler_WakeUp(void)
{
    Time time;
    TimeService_getTime(&time);
    
   for (int i = 0; i < 256; i++){
        switch (events[i].time.dayOfWeek)
        {
        case EVERYDAY:
            activation_helper(&events[i], time);
            break;
        case WEEKEND:
            if((time.dayOfWeek == SATURDAY || time.dayOfWeek == SUNDAY)){
                activation_helper(&events[i], time);
            }
            break;
        case WEEKDAY:
            if((time.dayOfWeek >= 1 && time.dayOfWeek <= 7)){
                activation_helper(&events[i], time);
            }
            break;
        default:
            if(events[i].time.dayOfWeek == time.dayOfWeek){
                activation_helper(&events[i], time);
            }
            break;
        }        
    }
}