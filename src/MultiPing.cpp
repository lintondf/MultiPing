#include <MultiPing.h>


#include <MultiPingTrace.h>

#if _MULTIPING_DEBUG_
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused
                                   // with << or >> streaming operators
#include <PrintEx.h>
#include <RunningStatistics.h>
#endif

namespace MultiPing {

void Sonar::recycle(unsigned long now) {
    if (usCycleTime == 0) {
        usecDelay = 1000;
    } else {
        unsigned long t = unsignedDistance(now, cycleStart);
        while (t > usCycleTime) t -= usCycleTime;
        usecDelay = usCycleTime - t;
    }
    whenEnqueued = now;
    waitEvent(false);
#if _MULTIPING_DEBUG_ > 2
    if (dbg)
        dbg->printf("%2d recycle %8lu %8lu %8lu\n", getId(), micros(), whenEnqueued, usecDelay);
#endif
    Task::getLongQueue().push_priority(this);
}

bool Sonar::start(unsigned long usStartDelay, unsigned long usCycleTime, Handler* handler) {
    this->handler = handler;
    this->usCycleTime = usCycleTime;
    state = IDLE;
    enqueueLong(usStartDelay);
    return true;
}

void Sonar::stop() {
    state = IDLE;
    waitEvent(false);
    if (slowQueue.contains(this)) slowQueue.erase(this);
    if (fastQueue.contains(this)) fastQueue.erase(this);
    device->reset();
}

#if _MULTIPING_DEBUG_
static const char* stateNames[] = {
    "IDLE",
    "START_PING",          // runs triggerStartPing()
    "WAIT_LAST_FINISHED",  // runs triggerWaitLastFinished()
    "WAIT_TRIGGER_PULSE",  // runs triggerWaitTriggerPulse()
    "WAIT_ECHO_STARTED",   // runs triggerWaitEchoStarted()
    "WAIT_ECHO",           // runs waitEchoComplete()
};
#endif

bool Sonar::dispatch(unsigned long now) {
#if _MULTIPING_DEBUG_ > 2
    if (state != IDLE)
        if (dbg)
            dbg->printf("%8lu dispatch %d: [%d] %s\n", now, getId(), (int)state,
                        stateNames[(int)state]);
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
    return false;
}

bool Sonar::triggerStartPing(unsigned long now) {
    // TRACE_TIMESTAMP( 1, getId() );
    // Serial.print(getId()); Serial.print(" ");
    // Serial.println( now % (3ul*device->usecMaxEchoDuration));
    device->begin();
    state = States::WAIT_LAST_FINISHED;
    enqueueShort(device->usecWaitEchoLowTimeout);
    return true;
}

bool Sonar::triggerWaitLastFinished(unsigned long now) {
    if (device->isEchoing()) {  // Previous ping hasn't finished, abort.
#if _MULTIPING_DEBUG_
        if (dbg) dbg->printf("Failed %8lu\n", now );
#endif
        if (handler) handler->error(this, STILL_PINGING);
        state = States::START_PING;
        recycle(now);
        return true;
    }
    device->beginTrigger();
    state = States::WAIT_TRIGGER_PULSE;
    enqueueShort(device->usecTriggerPulseDuration);
    return true;
}

bool Sonar::triggerWaitTriggerPulse(unsigned long now) {
    device->finishTrigger();
    timeout = micros() + device->usecMaxEchoStartDelay;
    // Maximum time we'll wait for ping to start
    state = States::WAIT_ECHO_STARTED;
    waitEvent(true);
#if _MULTIPING_DEBUG_ > 1
    if (dbg) dbg->printf("waiting trigger %d\n", getId());
#endif
    return true;
}

bool Sonar::triggerWaitEchoStarted(unsigned long now) {
    if (!device->isEchoing()) {                // Wait for ping to start.
        if (lessThanUnsigned(timeout, now)) {  // Took too long to start, abort.
#if _MULTIPING_DEBUG_
            if (dbg) dbg->printf("start failed: %8lu vs %8lu\n", now, timeout);
#endif
            if (handler) handler->error(this, PING_FAILED_TO_START);
            state = States::START_PING;
            device->reset();
            recycle(now);
            return true;
        }
        return false;
    }
    now = micros();
    echoStart = now;
    timeout = now + device->usecMaxEchoDuration;
#if _MULTIPING_DEBUG_ > 1
    if (dbg) dbg->printf("echo start %2d %8lu %8lu\n", getId(), now, timeout);
#endif
    state = States::WAIT_ECHO;
    waitEvent(true);
    return true;
}

bool Sonar::waitEchoComplete(unsigned long now) {
    now = micros();
    if (lessThanUnsigned(timeout, now)) {  // Took too long to finish, abort.
#if _MULTIPING_DEBUG_
        if (dbg)
            dbg->printf("no return %2d %8lu %8lu %8lu\n", getId(), now, timeout,
                        unsignedDistance(now, echoStart));
#endif
        if (handler) handler->event(this, NO_PING);
        device->reset();
        state = States::START_PING;
        recycle(now);
        return true;
    }
    if (device->isEchoing()) {                 // Wait for ping to finished
        return false;
    }
#if _MULTIPING_DEBUG_ > 1
    if (dbg)
        dbg->printf("echo rx %2d %8lu %8lu %8lu\n", getId(), now, timeout,
                    unsignedDistance(now, echoStart));
#endif
    unsigned long oneWay = unsignedDistance(now, echoStart) / 2;
    if (oneWay > 40000ul) {
        // Serial.print( oneWay); Serial.print(" "); 
        // Serial.print( echoStart); Serial.print(" "); 
        // Serial.print( now ); Serial.print(" "); 
        // Serial.print(   device->usecMaxEchoDuration );      
        // Serial.println();
    }
    if (handler) handler->event(this, oneWay);  // report one-way time
    state = States::START_PING;
    recycle(now);
    return true;
}

unsigned long Sonar::check() {
    device->begin();
    delayMicroseconds(device->usecWaitEchoLowTimeout);
    if (device->isEchoing()) {
        device->reset();
        return STILL_PINGING;
    }
    device->beginTrigger();
    delayMicroseconds(device->usecTriggerPulseDuration);
    device->finishTrigger();
    unsigned long timeout = micros() + device->usecMaxEchoStartDelay;
    while (!device->isEchoing()) {
        if (lessThanUnsigned(timeout, micros())) {  // Took too long to finish, abort.
            device->reset();
            return PING_FAILED_TO_START;
        }
    }
    timeout = micros() + device->usecMaxEchoDuration;
    unsigned long now = micros();
    while (device->isEchoing()) {
        if (lessThanUnsigned(timeout, now)) {  // Took too long to finish, abort.
            device->reset();
            return NO_PING;
        }
        now = micros();
    }
    return unsignedDistance(now, micros() - device->usecMaxEchoDuration);
}

}  // namespace MultiPing