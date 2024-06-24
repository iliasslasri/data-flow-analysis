#include "LightScheduler.h"
#include "unity.h"
#include "MockTimeService.h"

//// ------------------ Helper Functions ------------------ ////
void setUp(void)
{
    TimeService_startPeriodicAlarm_ExpectAnyArgsAndReturn(1);
    LightScheduler_Create();
}

void tearDown(void)
{
    TimeService_stopPeriodicAlarm_ExpectAnyArgs();
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
    TEST_ASSERT_EQUAL(1, LightScheduler_Create());
    LightScheduler_Destroy();
}

/// test desctruction of all events
void test_LightScheduler_Destroy(void)
{
    TimeService_startPeriodicAlarm_ExpectAnyArgsAndReturn(1);
    TimeService_stopPeriodicAlarm_ExpectAnyArgs();
    TEST_ASSERT_EQUAL(1, LightScheduler_Create());
    TEST_ASSERT_EQUAL(0, LightScheduler_Destroy());
}

/// test 2
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

/// test 4 : schedule multiple events
void test_LightScheduler_AddEvent_And_TurningOn_MultipleEvents(void)
{
    setUp();
    Time time1 = {FRIDAY, 1100};
    LightScheduler_AddEvent(1, time1, LIGHT_ON);

    Time time2 = {MONDAY, 1120};
    LightScheduler_AddEvent(2, time2, LIGHT_ON);

    getTime_And_WakeUp(time1);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastLightId());

    getTime_And_WakeUp(time2);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());
    TEST_ASSERT_EQUAL(2, LightControlSpy_getLastLightId());

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


/// test 6
void test_LightScheduler_IdConstraints(void)
{
    setUp();
    Time time = {THURDSDAY, 1100};
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(-1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(-200, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(0, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(256, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(2, LightScheduler_AddEvent(255, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(270, time, LIGHT_ON));
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
    Time time = {EVERYDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.dayOfWeek = MONDAY;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.minuteOfDay = 201;
    getTime_And_WakeUp(time);
    
    TEST_ASSERT_EQUAL(0, LightControlSpy_getLastState());

    time.dayOfWeek = SATURDAY;
    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 10
void test_LightScheduler_Light_State_Unkonwn_When_Creating_Event(void){
    setUp();
    Time time = {EVERYDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_OFF);

    TEST_ASSERT_EQUAL(-1, LightControlSpy_getLastState());
    tearDown();
}

/// test 11
void test_LightScheduler_WeekendEvent(void){
    setUp();
    Time time = {WEEKEND, 200};
    LightScheduler_AddEvent(0, time, LIGHT_ON);

    time.dayOfWeek = SATURDAY;
    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}


/// test 12
void test_LightScheduler_WeekdayEvent(void){
    setUp();
    Time time = {WEEKDAY, 200};
    LightScheduler_AddEvent(0, time, LIGHT_ON);

    time.dayOfWeek = MONDAY;
    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 13
void test_LightScheduler_MultipleEvents_for_same_lightID(void){
    
    setUp();
    // event 1, for light 1
    Time time = {WEEKEND, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    // event 2, for light 1
    time.dayOfWeek = MONDAY;
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    time.dayOfWeek = MONDAY;
    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    time.dayOfWeek = SUNDAY;
    time.minuteOfDay = 200;
    getTime_And_WakeUp(time);

    TEST_ASSERT_EQUAL(1, LightControlSpy_getLastState());

    tearDown();
}

/// test 14
void test_LightScheduler_Same_Multiple_Events(void){
    setUp();
    Time time = {MONDAY, 200};
    LightScheduler_AddEvent(1, time, LIGHT_ON);

    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(1, time, LIGHT_ON));

    tearDown();
}

/// test 15
void test_LightScheduler_RemoveEvent(void){
    setUp();
    Time time = {MONDAY, 200};

    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(0, LightScheduler_RemoveEvent(1, time));

    tearDown();
}

/// test 16
void test_LightScheduler_RemoveEvent_With_Unknown_Event(void){
    setUp();
    Time time = {MONDAY, 200};
    TEST_ASSERT_EQUAL(1, LightScheduler_AddEvent(1, time, LIGHT_ON));
    TEST_ASSERT_EQUAL(0, LightScheduler_RemoveEvent(1, time));

    TEST_ASSERT_EQUAL(-1, LightScheduler_RemoveEvent(1, time));

    tearDown();
}

/// test 17
void test_LightScheduler_Maximum_Events(void){
    setUp();
    Time time = {MONDAY, 200};
    for (int i = 0; i < 256; i++){
        LightScheduler_AddEvent(0, time, LIGHT_ON);
    }

    TEST_ASSERT_EQUAL(-1, LightScheduler_AddEvent(0, time, LIGHT_ON));
    tearDown();
}