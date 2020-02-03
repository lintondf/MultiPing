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

#include <MultiPingTrace.h>

namespace MultiPing {
StreamEx* Trace::out = nullptr;
bool Trace::stopAfterDump = false;
int Trace::lowMemory = 0;
Trace::TraceBlock* Trace::head = nullptr;
Trace::TraceBlock* Trace::tail = nullptr;
Trace::States Trace::state = Trace::MULTIPING_TRACE_STOPPED;
};  // namespace MultiPing