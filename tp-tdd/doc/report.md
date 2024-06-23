# Report for TDD Exercise

#### Requirements

1. Test the intialisation of the LightScheduler :
    - init the LightScheduler
    - check if the number of scheduled events is 0
    - check if the STATE of the LightScheduler is "OFF"

2. The number of scheduled events is increased by one when a new event is scheduled.
    - init the LightScheduler
    - create a LightTimeEvent
    - check if the number of scheduled events is 1
    - create a LightTimeEvent
    - check if the number of scheduled events is 2

3. The LightScheduler allows to schedule the switching of a room light.

   - Init a LightScheduler
   - Init a LightTimeEvent with a specific time to turn on the light.
   - Check if the Lights are on at the scheduled time
   - Init a LightTimeEvent with a different id with a specific time to turn off the light.
   - Check if the Lights are off at the scheduled time

4. The lightScheduler allows to schedule multiple events.
    
        - Init a LightScheduler
        - Init multiple LightTimeEvents with different times to turn on the light.
        - Check if the Lights are on at the scheduled time

5. Each light is identified by an integer, when modifying the light, the correct light should be modified.

    The test we should write consists of the following steps:
    - Create a LightScheduler 
    - Create a LightTimeEvent with an id of "1" for instance
    - check if the Light modified (LastLightId) has an id of "1"

6. ID of lights is between 0 and 255

    - check if we can create a Light object with an id of ID > 255 and ID < 0 (this test should fail, the called to create this event should return -1 for instance).

7. The actual switching is taken care of by a separate light control driver.

    - Init a LightScheduler
    - Init a LightTimeEvent, with a specific time to turn on the light.
    - do nothing with the Light function, and wait for the LightScheduler to call the Light function.
    - check if the Light object is on in the appropriate time.
    - check if the Light object is off in the appropriate time.

8. The user can schedule an event to turn on a light at a specific time but just around 1 minute.

    - create a LightScheduler
    - create a LightTimeEvent with a specific time to turn on the light.
    - check if the Light is on at the scheduled time
    - check if the Light is off at the scheduled time + 1min

9. The user can schedule switching a light on or off every day at a specific time.

    - create a LightScheduler 
    - set an event to turn on the light at a spectific time every day. 
    - check if the Light is on at the specific time for each day of the week
    - check if the Light is off at the specific time for each day of the week

10. Check that the light didn't turn on when creating the LightTimeEvent.

    - create a LightScheduler 
    - create a Light
    - create a LightTimeEvent with a time
    - check that the Light object is off

11. The user can schedule switching a light on or off during weekends (Saturday and Sunday).
    - create a LightScheduler
    - create a Light 
    - iterate over the days of the weekend and schedule the Light object to turn on at XX for instance
    - check if the Light object is on at XX for each day of the weekend
    - check if the Light object is off at XX + 1min for each day of the weekend

12. The user can schedule switching a light on or off during week days (Monday till Friday).

    - create a LightScheduler object
    - create a Light object
    - iterate over the days of the week and schedule the Light object to turn on at XX for instance
    - check if the Light object is on at XX for each day of the week
    - check if the Light object is off at XX + 1min for each day of the week
  
13. The user can schedule multiple events for the same light.

    - create a LightScheduler
    - create LightTimeEvents for the same ID, with different times.
      - schedule the Light is turned on at XX for instance
      - schedule the Light object to turn off at YY, time different from XX
    - check if the Light object is on at XX
    - check if the Light object is off at XX + 1min
    - check if the Light object is off at YY
    - check if the Light object is on at YY + 1min

14. The user cannot schedule the same event twice

    - create a LightScheduler
    - create a LightTimeEvent with a specific time to turn on the light
    - check that the creating a second event with the same time fails

15. The user can remove a scheduled event.

    - create a LightScheduler
    - create a LightTimeEvent with a specific time to turn on the light
    - remove the scheduled event
    - check if the Light object is off at XX

16. Removing a non existing event returns -1 (just to notify the user that the event does not exist).

    - create a LightScheduler
    - create a LightTimeEvent with a specific time to turn on the light
    - remove the scheduled event
    - check that removing the event again returns -1
  
17. The user can schedule a maximum number of 256 events.

    - create a LightScheduler
    - create 256 events for the Light object
    - check that creating a 257th event fails