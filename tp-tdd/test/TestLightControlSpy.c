#include "LightControlSpy.h"
#include "unity.h"

void testLightControlSpyReturnsLastStateChange(void)
{
    LightControl_init();

    LightControl_on(42);
   
    TEST_ASSERT_EQUAL( 42, LightControlSpy_getLastLightId() );
    TEST_ASSERT_EQUAL( LIGHT_ON, LightControlSpy_getLastState() );

    LightControl_destroy();
}


void test_LightControl_State_Unknown_After_Init(void)
{
    LightControl_init();

    TEST_ASSERT_EQUAL( LIGHT_STATE_UNKNOWN, LightControlSpy_getLastState() );

    LightControl_destroy();
}


void test_LightControl_Light_Id_Unknown_After_Init(void)
{
    LightControl_init();

    TEST_ASSERT_EQUAL( LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId() );

    LightControl_destroy();
}


void test_LightControl_OFF(void)
{
    LightControl_init();

    LightControl_off(42);
    TEST_ASSERT_EQUAL( LIGHT_OFF, LightControlSpy_getLastState() );

    LightControl_destroy();
}