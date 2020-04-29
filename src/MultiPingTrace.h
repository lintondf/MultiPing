#ifndef _MULTIPING_TRACE_H
#define _MULTIPING_TRACE_H 1

#ifndef PRINTEX_VERSION
#include <PrintEx.h>
#endif

#ifndef MULTIPING_TRACE
#define MULTIPING_TRACE true
#endif
#ifndef MULTIPING_TRACE_SERIAL
#define MULTIPING_TRACE_SERIAL \
    if (out != nullptr) out
#endif


namespace MultiPing {


class Trace {
   public:
    static void trace_start(StreamEx* out) {
        if (state == MULTIPING_TRACE_DISABLED) return;
        Trace::out = out;
        lowMemory = freeMemory() / 2;
        out->printf("TRACE_STARTED %d\n", lowMemory*2);
        state = MULTIPING_TRACE_STARTED;
    }
    static void trace_stopAfterDump(void (*callback)(void)) {
        stopAfterDump = callback;
    }
    static void trace_stop() {
        if (state == MULTIPING_TRACE_DISABLED) return;
        state = MULTIPING_TRACE_STOPPED;
    }
    static void trace_disable() {
        out->println("TRACE_DISABLED");
        state = MULTIPING_TRACE_DISABLED;
    }
    static void trace_enable() {
        out->println("TRACE_ENABLED");
        state = MULTIPING_TRACE_STOPPED;
    }

    static inline void trace_timestamp(uint8_t major, uint8_t minor) {
        append(major, minor, TIMESTAMP, micros() );
    }

    static inline void trace_value(uint8_t major, uint8_t minor, uint32_t value) {
        append(major, minor, VALUE, value);
    }

    static inline void trace_pointer(uint8_t major, uint8_t minor, void* value) {
        append(major, minor, POINTER, (uint32_t)value);
    }

    static void trace_dump();

   protected:
    Trace() {}  // pure static
    Trace( const Trace& that) {};

    static StreamEx* out;

    typedef enum {
        MULTIPING_TRACE_STOPPED,
        MULTIPING_TRACE_STARTED,
        MULTIPING_TRACE_DISABLED
    } States;
    static States state;

    static const int ENTRIES_PER_BLOCK = 100;

    typedef enum {
        TIMESTAMP,
        VALUE,
        POINTER,
    } ValueKinds;

    typedef struct {
        uint8_t major : 8;
        uint8_t minor : 8;
        ValueKinds kind : 8;
        uint32_t value : 32;
    } TraceEntry;

    class TraceBlock {
       public:
        TraceBlock() : next(nullptr), n(0) {}

        struct TraceBlock* next;
        unsigned n;
        TraceEntry entries[ENTRIES_PER_BLOCK];
    };

    static void (*stopAfterDump)(void);
    static int lowMemory;
    static TraceBlock* head;
    static TraceBlock* tail;

    static void append(uint8_t major, uint8_t minor, ValueKinds kind, uint32_t value);

    static int freeMemory();
};

#if MULTIPING_TRACE
#define TRACE_DUMP \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_dump();
#define TRACE_START(out) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_start(out);
#define TRACE_STOP \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_stop();
#define TRACE_STOPAFTERDUMP(c) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_stopAfterDump(c);

#define TRACE_DISABLE \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_disable();
#define TRACE_ENABLE \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_enable();

#define TRACE_TIMESTAMP(m, p) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_timestamp(m, p);
#define TRACE_VALUE(m, p, v) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_value(m, p, v);
#define TRACE_POINTER(m, p, v) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_pointer(m, p, v);
#else
#define TRACE_DUMP
#define TRACE_START
#define TRACE_STOP
#define TRACE_STOPAFTERDUMP(c)
#define TRACE_DISABLE
#define TRACE_ENABLE
#define TRACE_TIMESTAMP(m, p)
#define TRACE_VALUE(m, p, v)
#define TRACE_POINTER(m, p, v)
#endif

};      // namespace
#endif  //_MULTIPING_TRACE_H