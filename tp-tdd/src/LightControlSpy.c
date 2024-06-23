#include "LightControlSpy.h"

static int lastLightId = LIGHT_ID_UNKNOWN;
static int lastState = LIGHT_STATE_UNKNOWN;

void LightControl_init(void){
    lastLightId = LIGHT_ID_UNKNOWN;
    lastState = LIGHT_STATE_UNKNOWN;
}

void LightControl_on(int id){
    lastLightId = id;
    lastState = LIGHT_ON;
}

void LightControl_off(int id){
    lastLightId = id;
    lastState = LIGHT_OFF;
}

void LightControl_destroy(void){
    lastLightId = LIGHT_ID_UNKNOWN;
    lastState = LIGHT_STATE_UNKNOWN;
}

int LightControlSpy_getLastLightId(void) {
    return lastLightId;
}

int LightControlSpy_getLastState(void) {
    return lastState;
}
