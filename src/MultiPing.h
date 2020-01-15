#ifndef MULTIPING_H
#define MULTIPING_H 1
/***
 * MultiPing - Arduino Ultrasonic Sensor Library
 * (C) Copyright 2019 - Blue Lightning Development, LLC.
 * D. F. Linton. support@BlueLightningDevelopment.com
 *
 * SPDX-License-Identifier: MIT
 * See separate LICENSE file for full text
 */

/*********************************************************************
 * A library for handling multiple ultrasonic sensors via multitasking
 *********************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#if defined(PARTICLE)
#include <SparkIntervalTimer.h>
#else
#include <pins_arduino.h>
#endif
#endif

#if defined(__AVR__)
#include <avr/interrupt.h>
#include <avr/io.h>
//#include <avr/pgmspace.h>
#endif

#include <MultiPingUnits.h>
#include <Task.h> //TODO  ->MultiPingTask
#include <MultiPingDevice.h>

namespace MultiPing {

class Sonar : public Task {
   public:
    class Handler {
       public:
        Handler(){};
        virtual ~Handler(){};
        virtual void event(Sonar* task, long usecOneWay) = 0;
        virtual void error(Sonar* task, long errorCode) {
            event(task, errorCode);  // default is to handle errors as events
        }
    };
    Sonar(int id, Device* device)
        : Task(id),
          device(device),
          handler(nullptr),
          state(IDLE){

          };
    ~Sonar(){};

    enum ErrorCodes {
        NO_PING = 0L,                // device configured maximum echo return time elapsed
        STILL_PINGING = -1L,         // prior ping never stopped; reset may help
        PING_FAILED_TO_START = -2L,  // no device connected or device hard failure
        PING_TOO_LONG = -3L,         // echo return time exceeded device maximum value
        N_ERRORS = 3
    };

    bool start(unsigned long usStartDelay, unsigned long usCycleTime,
               Handler* handler);
    void stop();

    /**
     * @returns NO_PING if echo detected or timed out; other error codes as normal
     */
    ErrorCodes check();

    bool dispatch(unsigned long now);

    const Device* device;

   protected:
    Handler* handler;
    unsigned long timeout;      // usec maximum wait for event
    unsigned long echoStart;    // micros() at start of echo
    unsigned long cycleStart;   // micros() at start of cycle
    unsigned long usCycleTime;  // usec duration of sensor cycle
    enum States {
        IDLE,
        START_PING,          // runs triggerStartPing()
        WAIT_LAST_FINISHED,  // runs triggerWaitLastFinished()
        WAIT_TRIGGER_PULSE,  // runs triggerWaitTriggerPulse()
        WAIT_ECHO_STARTED,   // runs triggerWaitEchoStarted()
        WAIT_ECHO
    };  // runs waitEchoComplete()
    States state;

    void recycle(unsigned long now);

    bool triggerStartPing(unsigned long now);
    bool triggerWaitLastFinished(unsigned long now);
    bool triggerWaitEchoStarted(
        unsigned long now);  // after ECHO high -> "wait echo complete"; invokes
                             // errorFunc otherwise
    bool triggerWaitTriggerPulse(unsigned long now);
    bool waitEchoComplete(
        unsigned long now);  // invokes echoFunction unless times out; invokes
                             // errorFunc otherwise

};  // Sonar
}  // namespace MultiPing
#endif  // MULTIPING_H