#ifndef MULTIPING_H
#define MULTIPING_H 1

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

#include <Task.h>

namespace MultiPing {

#include <GPIO.h>

class Units {
   public:
    static inline unsigned long s2us(unsigned long ms) {
        return ms * 1000000ul;
    }
    static inline unsigned long ms2us(unsigned long ms) { return ms * 1000ul; }

    static void setTemperature(int dC);

    static inline unsigned long us2mm(unsigned long us) {
        return (((unsigned long)speedOfSound[iSoS].ms) * us) / 1000L +
               (((unsigned long)speedOfSound[iSoS].mms) * (us / 1000L)) / 1000L;
    }

    static float us2m(unsigned long us);
    static float us2cm(unsigned long us);
    static float us2ft(unsigned long us);
    static float us2in(unsigned long us);

   protected:
    static int cT;  // C temperature
    static const int nSoS = 17;
    typedef struct {
        int ms;
        int mms;
    } SoS_t;  // m/s, remainder mm/s
    // indexed by T(C)/5; T in range -30 to 50
    static const SoS_t speedOfSound[nSoS];
    static int iSoS;
};

class Device {
   public:
    Device() {}
    virtual void reset(IGPIO& trigger, IGPIO& echo) {
        trigger.low();
        echo.output();
        echo.low();
    }
    unsigned int usecWaitEchoLowTimeout = 10u;
    unsigned int usecTriggerPulseDuration = 24u;
    unsigned long usecMaxEchoStartDelay = 500u;
    unsigned long usecMaxEchoDuration = 60000ul;

   protected:
};

class Sonar : public Task {
   private:
    static Device defaultDevice;

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
    Sonar(int id, IGPIO& trigger, IGPIO& echo, Device* device = &defaultDevice)
        : Task(id),
          device(device),
          trigger(trigger),
          echo(echo),
          handler(nullptr),
          state(IDLE){

          };
    ~Sonar(){};

    enum ErrorCodes {
        NO_PING = 0L,
        STILL_PINGING = -1L,
        PING_FAILED_TO_START = -2L,
        N_ERRORS = 3  
    };

    bool start(unsigned long usStartDelay, unsigned long usCycleTime,
               Handler* handler);
    void stop();

    void calibrate();

    bool dispatch(unsigned long now);

    const Device* device;

   protected:
    IGPIO& trigger;
    IGPIO& echo;
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