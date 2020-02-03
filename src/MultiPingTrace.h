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

#ifdef __arm__
        // should use uinstd.h to define sbrk but Due causes a conflict
        extern "C" char* sbrk(int incr);
#else   // __ARM__
        extern char* __brkval;
#endif  // __arm__

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
    static void trace_stopAfterDump() {
        stopAfterDump = true;
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

    static void trace_timestamp(uint8_t module, uint8_t point) {
        TraceEntry* entry = append();
        if (entry == nullptr) return;
        entry->module = module;
        entry->point = point;
        entry->kind = TIMESTAMP;
        entry->value = micros();
    }

    static void trace_value(uint8_t module, uint8_t point, uint32_t value) {
        TraceEntry* entry = append();
        if (entry == nullptr) return;
        entry->module = module;
        entry->point = point;
        entry->kind = VALUE;
        entry->value = value;
    }

    static void trace_pointer(uint8_t module, uint8_t point, void* value) {
        TraceEntry* entry = append();
        if (entry == nullptr) return;
        entry->module = module;
        entry->point = point;
        entry->kind = POINTER;
        entry->value = (uint32_t)value;
    }

    static void trace_dump() {
        if (state != MULTIPING_TRACE_STARTED) return;

        TraceBlock* block = head;
        out->printf("TRACE_DUMP %d\n", freeMemory() );
        uint32_t lastTime;
        while (block != nullptr) {
            for (unsigned i = 0; i < block->n; i++) {
                switch (block->entries[i].kind) {
                    case TIMESTAMP:
                        lastTime = block->entries[i].value;
                        MULTIPING_TRACE_SERIAL->printf("T %10lu %3u %3u\n", lastTime,
                                                       block->entries[i].module,
                                                       block->entries[i].point);
                        break;
                    case VALUE:
                        MULTIPING_TRACE_SERIAL->printf(
                            "V %10lu %3u %3u %10lu\n", lastTime, block->entries[i].module,
                            block->entries[i].point, block->entries[i].value);
                        break;
                    case POINTER:
                        MULTIPING_TRACE_SERIAL->printf(
                            "P %10lu %3u %3u %10lx\n", lastTime, block->entries[i].module,
                            block->entries[i].point, block->entries[i].value);
                        break;
                }
            }
            block = block->next;
        }
        block = head;
        while (block != nullptr) {
            TraceBlock* b = block;
            block = block->next;
            delete b;
        }
        if (stopAfterDump) {
            out->printf("STOPPED AFTER DUMP %d\n", freeMemory() );
            while(true) ;
        }
    };

   protected:
    Trace() {}  // pure static

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
        uint8_t module : 8;
        uint8_t point : 8;
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

    static bool stopAfterDump;
    static int lowMemory;
    static TraceBlock* head;
    static TraceBlock* tail;

    static TraceEntry* append() {
        if (state != MULTIPING_TRACE_STARTED) return nullptr;
        if (freeMemory() < lowMemory) {
            trace_dump();  // flush and release blocks
        }
        if (head == nullptr) {
            head = new TraceBlock();
            tail = head;
        }
        if (tail == nullptr) return nullptr;  // out of memory
        TraceEntry* current = &tail->entries[tail->n++];
        if (tail->n >= ENTRIES_PER_BLOCK) {
            TraceBlock* empty = new TraceBlock();
            tail->next = empty;
            tail = empty;
        }
        return current;
    }

    static int freeMemory() {
        char top;
#ifdef __arm__
        return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
        return &top - __brkval;
#else   // __arm__
        return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
    }
};

#if MULTIPING_TRACE
#define TRACE_DUMP \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_dump();
#define TRACE_START(out) \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_start(out);
#define TRACE_STOP \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_stop();
#define TRACE_STOPAFTERDUMP \
    if (MULTIPING_TRACE) ::MultiPing::Trace::trace_stopAfterDump();

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
#define TRACE_STOPAFTERDUMP 
#define TRACE_DISABLE
#define TRACE_ENABLE
#define TRACE_TIMESTAMP(m, p)
#define TRACE_VALUE(m, p, v)
#define TRACE_POINTER(m, p, v)
#endif

};      // namespace
#endif  //_MULTIPING_TRACE_H