#include <MultiPing.h>

#define DEBUG 1 // 5288/505 vs 8038/694
#if DEBUG
#include <RunningStatistics.h>
#endif


namespace MultiPing {

    std::list<Task*>     Task::waiting;
    PriorityQueue<Task*> Task::fastQueue(Task::lessThan);
    PriorityQueue<Task*> Task::slowQueue(Task::lessThan);

    unsigned long        Task::cycleCount = 0ul;

    void Task::print(unsigned long now) {
#if DEBUG > 0
        for (PriorityQueue<Task*>::iterator it = slowQueue.begin(); it != slowQueue.end(); it++) {
            printf("%3d slowQueue %8lu %8lu (%8lu) %8lu %s\n", (*it)->getId(), now, (*it)->whenEnqueued, (now - (*it)->whenEnqueued), (*it)->usecDelay,
                (ready(now, (*it)) ? "T" : "F") );        
        }
        for (PriorityQueue<Task*>::iterator it = fastQueue.begin(); it != fastQueue.end(); it++) {
            printf("%3d fastQueue %8lu %8lu (%8lu) %8lu %s\n", (*it)->getId(), now, (*it)->whenEnqueued, (now - (*it)->whenEnqueued), (*it)->usecDelay,
                (ready(now, (*it)) ? "T" : "F") );        
        }
#endif        
    }

    void Task::run() {
        unsigned long now = micros();
#if DEBUG > 0
        if (cycleCount == 0L) {
            print(now);
        }
#endif    
        cycleCount++;
        if (DEBUG > 3) printf("run %8lu %8lu %u:%d/%d:%d/%d\n", now, cycleCount, waiting.size(), 
            slowQueue.size(), slowQueue.isEmpty(), fastQueue.size(), fastQueue.isEmpty() );
        if (!slowQueue.isEmpty()) {
            // if the top is not ready neither is anyone else
            if (ready(now, slowQueue.peek())) {
                now = micros();
                if (DEBUG >= 2) printf("slow %8lu/%8lu: %d:%d\n", now, cycleCount, slowQueue.size(), fastQueue.size() );
                slowQueue.pop()->dispatch(now);
            }
        }
        for (std::list<Task*>::iterator it = waiting.begin(); it != waiting.end(); ) {
            if ((*it)->dispatch(now)) {
                if (DEBUG >= 2) printf("wait %8lu/%8lu: %d\n", now, cycleCount, waiting.size());
                it = waiting.erase(it);
            } else {
                it++;
            }
        }
        now = micros();
        // work off short delay events until exhausted
        while (!fastQueue.isEmpty()) {
            if (!ready(now, fastQueue.peek())) {
                unsigned int dt = usecRemaining(now, fastQueue.peek());
                delayMicroseconds(dt);
            }
            now = micros();
            if (DEBUG > 2) printf("fast %8lu/%8lu: %d\n", now, cycleCount, fastQueue.size());
            fastQueue.pop()->dispatch(now);
        }
    }

    void Task::enqueueShort( Task* task ) {
        fastQueue.push(task);
    }

    void Task::enqueueLong( Task* task ) {
        slowQueue.push(task);
    }

    void Task::wait( Task* task ) {
        waiting.push_back(task);
    }

    Device Sonar::defaultDevice;

    void Sonar::enqueueShort(unsigned long usecDelay) {
        this->whenEnqueued = micros();
        this->usecDelay = usecDelay;
        Task::enqueueShort( this );
    }

    void Sonar::enqueueLong(unsigned long usecDelay) {
        this->whenEnqueued = micros();
        this->usecDelay = usecDelay;
        //printf("enqueue %p %8lu %8lu\n", this, this->whenEnqueued, this->usecDelay );
        Task::enqueueLong( this );
    }

    void Sonar::waitEvent() {
        Task::wait(this);
    }

    void Sonar::recycle(unsigned long now) {
        unsigned long t = unsignedDistance( now, cycleStart );
        while (t > usCycleTime)
            t -= usCycleTime;
        usecDelay = usCycleTime - t;
        whenEnqueued = now;
        if (DEBUG > 2) printf("recycle %8lu %8lu %8lu\n", micros(), whenEnqueued, usecDelay );
        Task::enqueueLong(this);
    }

    bool Sonar::start(unsigned long usStartDelay, unsigned long usCycleTime, Handler* handler ) {
        this->handler = handler;
        this->usCycleTime = usCycleTime;
        state = IDLE;
        enqueueLong(usStartDelay);
    }

    void Sonar::stop() {
        state = IDLE;
        // TODO remove from all Task lists
        device.reset(trigger, echo);
    }

    #if DEBUG
    static const char* stateNames[] = {
        "IDLE",
        "START_PING", // runs triggerStartPing()
        "WAIT_LAST_FINISHED", // runs triggerWaitLastFinished()
        "WAIT_TRIGGER_PULSE", // runs triggerWaitTriggerPulse()
        "WAIT_ECHO_STARTED",  // runs triggerWaitEchoStarted()
        "WAIT_ECHO",         // runs waitEchoComplete()
    };
    #endif

    bool Sonar::dispatch(unsigned long now) {
    #if DEBUG
        if (state != IDLE && DEBUG > 1) printf("%8lu dispatch %d: [%d] %s\n", now, getId(), (int)state, stateNames[(int)state] );
    #endif
        switch (state) {
            case IDLE:  // first cycle only
                cycleStart = now;
                state = START_PING;
            case START_PING:
                return triggerStartPing(now);
            case WAIT_LAST_FINISHED:
                return triggerWaitLastFinished(now);
            case WAIT_TRIGGER_PULSE:
                return triggerWaitTriggerPulse(now);
            case WAIT_ECHO_STARTED:
                return triggerWaitEchoStarted(now);
            case WAIT_ECHO:
                return waitEchoComplete(now);
        }
    }

    bool Sonar::triggerStartPing(unsigned long now) {
        echo.input();
        echo.pullup();
        state = States::WAIT_LAST_FINISHED;
        enqueueShort( device.usecWaitEchoLowTimeout );
        return true;
    }

    bool Sonar::triggerWaitLastFinished(unsigned long now) {
        if (echo.read()) {  // Previous ping hasn't finished, abort.
            printf("Failed %8lu: %u\n", now, echo.read());
            if (handler) handler->error(this, STILL_PINGING);
            state = States::START_PING;
            recycle(now);
            return true;
        }
        trigger.output();
        trigger.low();
        trigger.high();  // Set trigger pin high, this tells the sensor to send out a ping.
        state = States::WAIT_TRIGGER_PULSE;
        enqueueShort( device.usecTriggerPulseDuration );
        return true;
    }

    bool Sonar::triggerWaitTriggerPulse(unsigned long now) {
        trigger.low();
    #if ONE_PIN_ENABLED == true
        if (trigger.getPin() == echo.getPin()) {
            echo.input();
    #if INPUT_PULLUP == true
            echo.pullup();
    #endif
        }
    #endif
        timeout = micros() + device.usecMaxEchoDuration + device.usecMaxEchoStartDelay; 
                       // Maximum time we'll wait for ping to start
                                        // (most sensors are <450uS, the SRF06 can take
                                        // up to 34,300uS!)
        state = States::WAIT_ECHO_STARTED;
        waitEvent();
        return true;
    }

    bool Sonar::triggerWaitEchoStarted(unsigned long now) {
        if (!echo.read()) {              // Wait for ping to start.
            if (lessThanUnsigned( timeout, now)) {  // Took too long to start, abort.
                if (DEBUG) printf("start failed: %8lu vs %8lu\n", now, timeout );
                if (handler) handler->error(this, PING_FAILED_TO_START);
                state = States::START_PING;
                device.reset(trigger, echo);
                recycle(now);
                return true;
            }
            return false;
        }
        echoStart = now;
        timeout = now + device.usecMaxEchoDuration;
        state = States::WAIT_ECHO;
        waitEvent();
        return true;
    }

    bool Sonar::waitEchoComplete(unsigned long now) {
        if (echo.read()) {              // Wait for ping to fished
            if (lessThanUnsigned( timeout, now)) {  // Took too long to finish, abort.
                if (DEBUG) printf("no echo %2d %8lu %8lu %8lu\n", getId(), now, timeout, unsignedDistance(now, echoStart) );
                if (handler) handler->event(this, NO_PING);
                device.reset(trigger, echo);
                state = States::START_PING;
                recycle(now);
                return true;
            }
            return false;
        }
        if (handler) handler->event(this, now - echoStart);
        state = States::START_PING;
        recycle(now);
        return true;
    }

} // namespace MultiPing