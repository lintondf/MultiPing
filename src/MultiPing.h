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
#endif

#include <ArduinoSTL.h>
#include <list>
#include <PriorityQueue.h>

namespace MultiPing {

#include <GPIO.h>


    inline int compareUnsigned( unsigned long a, unsigned long b ) {
        unsigned long d0 = a - b;
        //printf("d0 %8lu\n", d0);
        if (d0 == 0ul)
            return 0;
        unsigned long d1 = b - a;
        //printf("d1 %8lu\n", d1);
        return (d0 < d1) ? +1 : -1;
    }

    inline unsigned long lessThanUnsigned( unsigned long a, unsigned long b ) {
        return compareUnsigned( a, b ) == -1;
    }

    inline unsigned long unsignedDistance( unsigned long a, unsigned long b ) {
        return std::min( a-b, b-a );
    }


    class Units {
        public:
            static inline unsigned long s2us( unsigned long ms) {
                return ms * 1000000ul;
            }
            static inline unsigned long ms2us( unsigned long ms) {
                return ms * 1000ul;
            }

            static void setTemperature( int dC );

            static inline unsigned long us2mm( unsigned long us ) {
                //printf("%lu %d %d %d\n", us, iSoS, speedOfSound[iSoS].ms, speedOfSound[iSoS].mms );
                //printf(" %lu %lu\n", (((unsigned long)speedOfSound[iSoS].ms)*us),
                //    (((unsigned long)speedOfSound[iSoS].mms)*(us/1000L)) );
                return  (((unsigned long)speedOfSound[iSoS].ms)*us) / 1000L + 
                        (((unsigned long)speedOfSound[iSoS].mms)*(us/1000L)) / 1000L;
            }

            static float us2m( unsigned long us );
            static float us2cm( unsigned long us );
            static float us2ft( unsigned long us );
            static float us2in( unsigned long us );
        protected:
            static int cT; // C temperature
            static const int nSoS = 17;
            typedef struct {int ms; int mms;} SoS_t;
            // indexed by T(C)/5; T in range -30 to 50
            //   [0] m/s, [1] remainder mm/s  
            static SoS_t speedOfSound[nSoS];
            static int iSoS;
    };

    class Task {
        public:
            Task(int id) : id(id) {}
            virtual ~Task() {}
            virtual int getId() {
                return id;
            }
            virtual bool dispatch(unsigned long now);

            void enqueueShort(unsigned long usecDelay);
            void enqueueLong(unsigned long usecDelay);
            void waitEvent();

            static inline bool lessThan( Task* lhs, Task* rhs) {
                //printf("lessThan %p %2d %8lu %8lu\n", lhs, lhs->getId(), lhs->whenEnqueued, lhs->usecDelay );
                //printf("         %p %2d %8lu %8lu\n", rhs, rhs->getId(), rhs->whenEnqueued, rhs->usecDelay );
                bool result = lessThanUnsigned( lhs->whenEnqueued+lhs->usecDelay, rhs->whenEnqueued+rhs->usecDelay );
                //printf("         %s\n", (result) ? "T":"F");
                return result;
            }

            static void run();
            static void print(unsigned long now);
            static void enqueueShort( Task* task );
            static void enqueueLong( Task* task );
            static void wait( Task* task );

        protected:
            int id;
            unsigned long whenEnqueued;  // usec (from micros())
            unsigned long usecDelay;     // usec wait requested

            static bool ready( unsigned long now, Task* task) {
                return ! lessThanUnsigned( now, task->whenEnqueued + task->usecDelay );
            }

            // only valid is ! ready()
            static unsigned int usecRemaining(unsigned long now, Task *task) {
                unsigned long dt = (task->whenEnqueued + task->usecDelay) - now;
                return (unsigned int)dt;
            }

            static unsigned long        cycleCount;
            static std::list<Task*>     waiting;
            static PriorityQueue<Task*> fastQueue;
            static PriorityQueue<Task*> slowQueue;

    };

    class Device {
        public:
            Device() {}
            virtual void reset(IGPIO &trigger, IGPIO &echo) {
                trigger.low();
                echo.output();
                echo.low();
            }
            unsigned int  usecWaitEchoLowTimeout = 4u;
            unsigned int  usecTriggerPulseDuration = 16u;
            unsigned long usecMaxEchoStartDelay = 500u;
            unsigned long usecMaxEchoDuration = 50000ul;
        protected:
    };

    class Sonar : public Task {
        private:
            static Device defaultDevice;
        public:
            class Handler {
                public:
                    Handler() {};
                    virtual ~Handler() {};
                    virtual void event( Sonar* task, long usecOneWay ) = 0;
                    virtual void error( Sonar* task, long errorCode ) {
                        event( task, errorCode ); // default is to handle errors as events
                    }
            };
            Sonar(int id, IGPIO &trigger, IGPIO &echo, Device& device = defaultDevice ) : Task(id),
                device(device),
                trigger(trigger),
                echo(echo),
                handler(nullptr),
                state(IDLE)
            {

            };
            ~Sonar() {};

            enum ErrorCodes {
                NO_PING = 0L,
                STILL_PINGING = -1L,
                PING_FAILED_TO_START = -2L,
            };

            bool start(unsigned long usStartDelay, unsigned long usCycleTime, Handler* handler );
            void stop();

            bool dispatch(unsigned long now);

        protected:
            Device& device;
            IGPIO &trigger;
            IGPIO &echo;
            Handler* handler;
            unsigned long timeout;       // usec maximum wait for event
            unsigned long echoStart;     // micros() at start of echo
            unsigned long cycleStart;    // micros() at start of cycle
            unsigned long usCycleTime;   // usec duration of sensor cycle
            enum States { IDLE,
                        START_PING, // runs triggerStartPing()
                        WAIT_LAST_FINISHED, // runs triggerWaitLastFinished()
                        WAIT_TRIGGER_PULSE, // runs triggerWaitTriggerPulse()
                        WAIT_ECHO_STARTED,  // runs triggerWaitEchoStarted()
                        WAIT_ECHO };        // runs waitEchoComplete()
            States state;

            void enqueueShort(unsigned long usecDelay);
            void enqueueLong(unsigned long usecDelay);
            void waitEvent();
            void recycle(unsigned long now);

            bool triggerStartPing(unsigned long now);
            bool triggerWaitLastFinished(unsigned long now);
            bool triggerWaitEchoStarted(unsigned long now);  // after ECHO high -> "wait echo complete"; invokes errorFunc otherwise
            bool triggerWaitTriggerPulse(unsigned long now);
            bool waitEchoComplete(unsigned long now);        // invokes echoFunction unless times out; invokes errorFunc otherwise

    }; // Sonar
} // namespace MultiPing
#endif //MULTIPING_H