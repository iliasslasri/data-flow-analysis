#include "LightScheduler.h"
#include "unity.h"
#include "MockTimeService.h"

//// ------------------ Helper Functions ------------------ ////
void setUp(void)
{
    TimeService_startPeriodicAlarm_ExpectAnyArgsAndReturn(1);
    TimeService_init_Expect();
    LightScheduler_Create();
}

void tearDown(void)
{
    TimeService_stopPeriodicAlarm_ExpectAnyArgs();
    TimeService_destroy_Expect();
    LightScheduler_Destroy();
}

void getTime_And_WakeUp(Time time){
    TimeService_getTime_ExpectAnyArgs();
    TimeService_getTime_ReturnThruPtr_time(&time);
    LightScheduler_WakeUp();
}


//// ------------------ Tests ------------------ ////
/// test 1
void test_LightScheduler_Create(void)
{
    // To call LightScheduler_Create without passing by setUp and tearDown
    TimeService_startPeriodicAlarm_ExpectAnyArgsAndReturn(1);
    TimeService_stopPeriodicAlarm_ExpectAnyArgs();
    TimeService_init_Expect();

    TEST_ASSERT_EQUAL(1, LightScheduler_Create());

    TimeService_destroy_Expect();
    LightScheduler_Destroy();
}

/// test desctruction of all events
void test_LightScheduler_Destroy(void)
{
    TimeService_startPeriodicAlarm_ExpectAnyArgsAndReturn(1);
    TimeService_stopPeriodicAlarm_ExpectAnyArgs();
    TimeService_init_Expect();

    TEST_ASSERT_EQUAL(1, LightScheduler_Create());

    TimeService_destroy_Expect();

    TEST_ASSERT_EQUAL(1, LightScheduler_Destroy());
}

/// test 2 and 23 : because the number of events is increasing from 0
void test_LightScheduler_AddEvent_incresing_Number_of_events(void)
{
    setUp();
    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(1, (Time){THURDSDAY , 1100}, LIGHT_ON));
    
    TEST_ASSERT_EQUAL(2, LightScheduler_AddEvent(1, (Time){MONDAY, 1150}, LIGHT_ON));
    tearDown();
}

/// test 3
void test_LightScheduler_TurningOn(void)
{
    setUp();
    Time time = {FRIDAY, 110};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastLightId());

    // Another random time
    Time time2 = {THURDSDAY, 120};
    LightScheduler_AddEvent(2, time2, LIGHT_ON);

    getTime_And_WakeUp(time2);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(2, LightControlSpy_getLastLightId());
    
    tearDown();
}

void test_LightScheduler_TurningOff(void)
{
    setUp();
    Time time2 = {MONDAY, 120};
    LightScheduler_AddEvent(2, time2, LIGHT_OFF);

    getTime_And_WakeUp(time2);

    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(2, LightControlSpy_getLastLightId());

    tearDown();
}

/// test 4 : schedule multiple events, any day, any time
void test_LightScheduler_AddEvent_And_TurningOn_MultipleEvents(void)
{
    setUp();
    Time time1 = {FRIDAY, 10};
    LightScheduler_AddEvent(1, time1, LIGHT_ON);

    Time time2 = {MONDAY, 20};
    LightScheduler_AddEvent(2, time2, LIGHT_ON);

    Time time3 = {TUESDAY, 130};
    LightScheduler_AddEvent(3, time3, LIGHT_ON);

    Time time4 = {WEDNESDAY, 990};
    LightScheduler_AddEvent(4, time4, LIGHT_ON);

    Time time5 = {THURDSDAY, 500};
    LightScheduler_AddEvent(5, time5, LIGHT_ON);

    Time time6 = {FRIDAY, 110};
    LightScheduler_AddEvent(6, time6, LIGHT_ON);

    Time time7 = {SATURDAY, 0};
    LightScheduler_AddEvent(7, time7, LIGHT_ON);

    Time time8 = {SUNDAY, 500};
    LightScheduler_AddEvent(8, time8, LIGHT_ON);

    getTime_And_WakeUp(time1);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time2);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(2, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time3);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(3, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time4);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(4, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time5);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(5, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time6);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(6, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time7);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(7, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time8);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(8, LightControlSpy_getLastLightId());
    tearDown();
}

/// test 5 
void test_LightScheduler_checkingID(void)
{
    setUp();
    Time time = {FRIDAY, 1100};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastLightId());
    tearDown();
}


/// test 6.1
void test_LightScheduler_IdConstraints_Out_Of_Range(void)
{
    setUp();
    Time time = {THURDSDAY, 1100};
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(-1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(-200, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(256, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(270, time, LIGHT_ON));
    tearDown();
}

/// test 6.2
void test_LightScheduler_IDConstraints_In_Range(void)
{
    setUp();
    Time time = {THURDSDAY, 1100};
    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(255, time, LIGHT_ON));

    TEST_ASSERT_EQUAL(2, LightScheduler_AddEvent(0, time, LIGHT_ON));
    tearDown();
}

/// test 7
void test_LightScheduler_NoActionNeededtoTurnOn(void){
    setUp();
    Time time = {THURDSDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    getTime_And_WakeUp(time);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    tearDown();
}

/// test 8
void test_LightScheduler_1minute(void){

    setUp();
    Time time = {THURDSDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.minuteOfDay = 201;
    getTime_And_WakeUp(time);
    
    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());
    tearDown();
}

/// test 9
void test_LightScheduler_EveryDayEvent(void){
    setUp();
    Time time = {EVERYDAY, 1430};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.dayOfWeek = MONDAY;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.minuteOfDay = 1431;
    getTime_And_WakeUp(time);
    
    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());

    time.dayOfWeek = SATURDAY;
    time.minuteOfDay = 1430;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 10
void test_LightScheduler_Light_State_Unkonwn_When_Creating_Event(void){
    setUp();
    Time time = {EVERYDAY, 1200};
    LightScheduler_AddEvent(1, time, LIGHT_OFF);

    TEST_ASSERT_EQUAL(-1, LightControlSpy_getLastState());
    tearDown();
}

/// test 11
void test_LightScheduler_WeekendEvent(void){
    setUp();
    Time time = {WEEKEND, 900};
    LightScheduler_AddEvent(0, time, LIGHT_ON);

    time.dayOfWeek = SATURDAY;
    time.minuteOfDay = 900;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}


/// test 12
void test_LightScheduler_WeekdayEvent(void){
    setUp();
    Time time = {WEEKDAY, 500};
    LightScheduler_AddEvent(0, time, LIGHT_ON);

    time.dayOfWeek = MONDAY;
    time.minuteOfDay = 500;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 13
void test_LightScheduler_MultipleEvents_for_same_lightID(void){
    
    setUp();
    // event 1, for light 1
    Time time = {WEEKEND, 1200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    // event 2, for light 1
    time.dayOfWeek = MONDAY;
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.dayOfWeek = SUNDAY;
    time.minuteOfDay = 1200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 14
void test_LightScheduler_Same_Multiple_Events(void){
    setUp();
    Time time = {MONDAY, 1300};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(1, time, LIGHT_ON));

    tearDown();
}

/// test 15
void test_LightScheduler_RemoveEvent(void){
    setUp();
    Time time = {SUNDAY, 800};

    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(0, LightScheduler_RemoveEvent(1, time));

    tearDown();
}

/// test 16
void test_LightScheduler_RemoveEvent_With_Unknown_Event(void){
    setUp();
    Time time = {TUESDAY, 1000};
    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(0, LightScheduler_RemoveEvent(1, time));

    TEST_ASSERT_EQUAL(-1, LightScheduler_RemoveEvent(1, time));

    tearDown();
}

/// test 17
void test_LightScheduler_Maximum_Events(void){
    setUp();
    Time time = {THURDSDAY, 200};
    for (int i = 0; i < 256; i++){
        LightScheduler_AddEvent(0, time, LIGHT_ON);
    }

    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));
    tearDown();
}

/// test 18
void test_LightScheduler_Maximum_Events_With_Remove(void){
    setUp();
    Time time = {MONDAY, 0};
    for (int i = 0; i < 256; i++){
        LightScheduler_AddEvent(0, time, LIGHT_ON);
    }

    for (int i = 0; i < 256; i++){
        LightScheduler_RemoveEvent(0, time);
    }

    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(0, time, LIGHT_ON));
    tearDown();
}

/// test 19
void test_LightScheduler_Maximum_Events_With_Remove_Unknown_Event(void){
    setUp();
    Time time = {MONDAY, 0};
    for (int i = 0; i < 256; i++){
        LightScheduler_AddEvent(0, time, LIGHT_ON);
    }

    for (int i = 0; i < 256; i++){
        LightScheduler_RemoveEvent(0, time);
    }

    TEST_ASSERT_EQUAL(-1, LightScheduler_RemoveEvent(0, time));
    tearDown();
}

/// test 20 : minuteOfDay is out of range
void test_LightScheduler_MinuteOfDay_Out_Of_Range(void){
    setUp();
    Time time = {MONDAY, 1440};
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));

    time.minuteOfDay = -1;
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));
    tearDown();
}

/// test 21 : state_trigger is not LIGHT_ON or LIGHT_OFF
void test_LightScheduler_StateTrigger(void){
    setUp();
    Time time = {MONDAY, 200};
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, 2));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, -1));
    tearDown();
}

/// test 22 : dayOfWeek is not valid
void test_LightScheduler_DayOfWeek_Out_Of_Range(void){
    setUp();
    Time time = {0, 200};
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));

    time.dayOfWeek = 8;
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));

    time.dayOfWeek = -1;
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));

    time.dayOfWeek = 13;
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));

    tearDown();
}

/// test 24
void test_LightScheduler_Successive_Events_Same_Light_on(void){
    setUp();
    Time time = {MONDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.minuteOfDay = 201;
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.minuteOfDay = 201;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

void test_LightScheduler_Successive_Events_Same_Light_off(void){
    setUp();
    Time time = {4, 0};
    LightScheduler_AddEvent(1, time, LIGHT_OFF);

    time.minuteOfDay = 1;
    LightScheduler_AddEvent(1, time, LIGHT_OFF);

    time.minuteOfDay = 0;
    getTime_And_WakeUp(time);
    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());

    time.minuteOfDay = 1;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());

    tearDown();
}

void test_LightScheduler_Multiple_Events_Contradicting_States(void){
    setUp();
    Time time = {MONDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.minuteOfDay = 200;

    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(1, time, LIGHT_OFF));

    tearDown();
}