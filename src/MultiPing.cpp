#include <MultiPing.h>

#define DEBUG 0
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused
                                   // with << or >> streaming operators
#include <PrintEx.h>
#include <RunningStatistics.h>

namespace MultiPing {

Device Sonar::defaultDevice;

void Sonar::recycle(unsigned long now) {
    unsigned long t = unsignedDistance(now, cycleStart);
    while (t > usCycleTime) t -= usCycleTime;
    usecDelay = usCycleTime - t;
    whenEnqueued = now;
    waitEvent(false);
    if (DEBUG > 2)
        if (dbg)
            dbg->printf("%2d recycle %8lu %8lu %8lu\n", getId(), micros(),
                        whenEnqueued, usecDelay);
    Task::getLongQueue().push_priority(this);
}

bool Sonar::start(unsigned long usStartDelay, unsigned long usCycleTime,
                  Handler* handler) {
    this->handler = handler;
    this->usCycleTime = usCycleTime;
    state = IDLE;
    enqueueLong(usStartDelay);
}

void Sonar::stop() {
    state = IDLE;
    waitEvent(false);
    if (slowQueue.contains(this)) slowQueue.erase(this);
    if (fastQueue.contains(this)) fastQueue.erase(this);
    device->reset(trigger, echo);
}

#if DEBUG
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
#if DEBUG
    if (/*state != IDLE &&*/ DEBUG > 1)
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
}

bool Sonar::triggerStartPing(unsigned long now) {
    echo.input();
    echo.pullup();
    state = States::WAIT_LAST_FINISHED;
    enqueueShort(device->usecWaitEchoLowTimeout);
    return true;
}

bool Sonar::triggerWaitLastFinished(unsigned long now) {
    if (echo.read()) {  // Previous ping hasn't finished, abort.
        if (DEBUG)
            if (dbg) dbg->printf("Failed %8lu: %u\n", now, echo.read());
        if (handler) handler->error(this, STILL_PINGING);
        state = States::START_PING;
        recycle(now);
        return true;
    }
    trigger.output();
    trigger.low();
    trigger.high();  // Set trigger pin high, this tells the sensor to send out
                     // a ping.
    state = States::WAIT_TRIGGER_PULSE;
    enqueueShort(device->usecTriggerPulseDuration);
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
    timeout =
        micros() + device->usecMaxEchoDuration + device->usecMaxEchoStartDelay;
    // Maximum time we'll wait for ping to start
    // (most sensors are <450uS, the SRF06 can take
    // up to 34,300uS!)
    state = States::WAIT_ECHO_STARTED;
    waitEvent(true);
    // if (DEBUG) if (dbg) dbg->printf("waiting trigger %d\n", getId() );
    return true;
}

bool Sonar::triggerWaitEchoStarted(unsigned long now) {
    if (!echo.read()) {                        // Wait for ping to start.
        if (lessThanUnsigned(timeout, now)) {  // Took too long to start, abort.
            if (DEBUG)
                if (dbg)
                    dbg->printf("start failed: %8lu vs %8lu\n", now, timeout);
            if (handler) handler->error(this, PING_FAILED_TO_START);
            state = States::START_PING;
            device->reset(trigger, echo);
            recycle(now);
            return true;
        }
        return false;
    }
    now = micros();
    echoStart = now;
    timeout = now + device->usecMaxEchoDuration;
    if (DEBUG)
        if (dbg)
            dbg->printf("echo start %2d %8lu %8lu\n", getId(), now, timeout);
    state = States::WAIT_ECHO;
    waitEvent(true);
    return true;
}

bool Sonar::waitEchoComplete(unsigned long now) {
    now = micros();
    if (echo.read()) {  // Wait for ping to finished
        if (lessThanUnsigned(timeout,
                             now)) {  // Took too long to finish, abort.
            if (DEBUG)
                if (dbg)
                    dbg->printf("no return %2d %8lu %8lu %8lu\n", getId(), now,
                                timeout, unsignedDistance(now, echoStart));
            if (handler) handler->event(this, NO_PING);
            device->reset(trigger, echo);
            state = States::START_PING;
            recycle(now);
            return true;
        }
        return false;
    }
    if (DEBUG)
        if (dbg)
            dbg->printf("echo rx %2d %8lu %8lu %8lu\n", getId(), now, timeout,
                        unsignedDistance(now, echoStart));
    if (handler) handler->event(this, unsignedDistance(now, echoStart) / 2);
    state = States::START_PING;
    recycle(now);
    return true;
}

void Sonar::calibrate() {
    RunningStatistics stats, stat2;
    for (int i = 0; i < 100; i++) {
        trigger.output();
        trigger.low();
        // TODO single pin support
        echo.input();
        echo.pullup();
        while (echo.read())
            ;
        trigger.pulse(24);
        unsigned long waitStart = micros();
        while (!echo.read())
            ;
        unsigned long echoStart = micros();
        while (echo.read())
            ;
        unsigned long echoFinish = micros();
        stats.push(unsignedDistance(echoStart, waitStart));
        stat2.push(unsignedDistance(waitStart, echoFinish));
        delay(50);
    }
    if (dbg)
        dbg->printf(
            "%2d Trigger TE to Echo LE (us, mean, var, min, max): %10.0f "
            "%10.0f %10.0f %10.0f\n",
            getId(), stats.mean(), stats.variance(), stats.minimum(),
            stats.maximum());
    if (dbg)
        dbg->printf(
            "%2d Echo LE to Echo TE (us, mean, var, min, max):    %10.0f "
            "%10.0f %10.0f %10.0f\n",
            getId(), stat2.mean(), stat2.variance(), stat2.minimum(),
            stat2.maximum());
}

int Units::iSoS = (0 + 30) / 5;
int Units::cT = 0;
const PROGMEM Units::SoS_t Units::speedOfSound[nSoS] = {
    {312, 512}, {315, 709}, {318, 873}, {322, 7},   {325, 110}, {328, 185},
    {331, 230}, {334, 248}, {337, 239}, {340, 203}, {343, 142}, {346, 56},
    {348, 946}, {351, 812}, {354, 655}, {357, 475}, {360, 273}};

void Units::setTemperature(int dC) {
    dC = (-30 > dC) ? -30 : dC;
    dC = (+50 < dC) ? +50 : dC;
    cT = dC;
    iSoS = (dC + 30) / 5;
}

float Units::us2m(unsigned long us) {
    float dT = ((float)cT) - (5.0 * (float)iSoS - 30.0);
    if (iSoS + 1 < nSoS) {
        float a = (float)speedOfSound[iSoS].ms +
                  0.001 * (float)speedOfSound[iSoS].mms;
        float b = (float)speedOfSound[iSoS + 1].ms +
                  0.001 * (float)speedOfSound[iSoS + 1].mms;
        return a + dT * (b - a) / 5.0;
    } else {
        float a = (float)speedOfSound[iSoS - 1].ms +
                  0.001 * (float)speedOfSound[iSoS - 1].mms;
        float b = (float)speedOfSound[iSoS].ms +
                  0.001 * (float)speedOfSound[iSoS].mms;
        return b + dT * (b - a) / 5.0;
    }
}

float Units::us2cm(unsigned long us) { return 100.0 * us2m(us); }

float Units::us2ft(unsigned long us) { return us2m(us) / 0.3048; }

float Units::us2in(unsigned long us) { return 12.0 * us2ft(us); }

}  // namespace MultiPing