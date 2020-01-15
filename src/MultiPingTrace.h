#ifndef _MULTIPING_TRACE_H
#define _MULTIPING_TRACE_H 1

#ifndef MULTIPING_TRACE
#define MULTIPING_TRACE true
#endif
#ifndef MULTIPING_TRACE_SERIAL
#define MULTIPING_TRACE_SERIAL if (out != nullptr) out
#endif

namespace MultiPing {

    class Trace {
        public:

            static void trace_start(Stream* out) {
                if (state == MULTIPING_TRACE_DISABLED) return;
                Trace::out = out;
                state = MULTIPING_TRACE_STARTED;
            }
            static void trace_stop() {
                if (state == MULTIPING_TRACE_DISABLED) return;
                state = MULTIPING_TRACE_STOPPED;
            }
            static void trace_disable() { state = MULTIPING_TRACE_DISABLED; }
            static void trace_enable() { state = MULTIPING_TRACE_STOPPED; }

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
                uint32_t lastTime;
                char line[10 + 1 + 3 + 1 + 3 + 1 + 10 + 1];
                while (block != nullptr) {
                    for (unsigned i = 0; i < block->n; i++) {
                        switch (block->entries[i].kind) {
                            case TIMESTAMP:
                                lastTime = block->entries[i].value;
                                sprintf(line, "%10lu %3u %3u", lastTime,
                                        block->entries[i].module, block->entries[i].point );
                                MULTIPING_TRACE_SERIAL->println(line);
                                break;
                            case VALUE:
                                sprintf(line, "%10lu %3u %3u %10lu", lastTime,
                                        block->entries[i].module, block->entries[i].point,
                                        block->entries[i].value);
                                MULTIPING_TRACE_SERIAL->println(line);
                                break;
                            case POINTER:
                                sprintf(line, "%10lu %3u %3u %8lx  ", lastTime,
                                        block->entries[i].module, block->entries[i].point,
                                        block->entries[i].value);
                                uint32_t* p = (uint32_t*)block->entries[i].value;
                                MULTIPING_TRACE_SERIAL->print(line);
                                MULTIPING_TRACE_SERIAL->println(*p, HEX);
                                break;
                        }
                    }
                    block = block->next;
                }
            };

        protected:
            Trace() {}  // pure static 

            static Stream* out;

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

            static TraceBlock* head;
            static TraceBlock* tail;

            static TraceEntry* append() {
                if (state != MULTIPING_TRACE_STARTED) return nullptr;
                if (head == nullptr) {
                    head = new TraceBlock();
                    tail = head;
                }
                if (tail == nullptr) return nullptr;
                TraceEntry* current = &tail->entries[tail->n++];
                if (tail->n >= ENTRIES_PER_BLOCK) {
                    TraceBlock* empty = new TraceBlock();
                    tail->next = empty;
                    tail = empty;
                }
                return current;
            }

    };

#if MULTIPING_TRACE
#define TRACE_DUMP \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_dump();
#define TRACE_START(out) \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_start(out);
#define TRACE_STOP \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_start();
#define TRACE_DISABLE \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_disable();
#define TRACE_ENABLE \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_enable();

#define TRACE_TIMESTAMP(m, p) \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_timestamp(m, p);
#define TRACE_VALUE(m, p, v) \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_value(m, p, v);
#define TRACE_POINTER(m, p, v) \
    if (MULTIPING_TRACE) MultiPing::Trace::trace_pointer(m, p, v);
#else
#define TRACE_DUMP
#define TRACE_START
#define TRACE_STOP
#define TRACE_DISABLE
#define TRACE_ENABLE
#define TRACE_TIMESTAMP(m, p)
#define TRACE_VALUE(m, p, v)
#define TRACE_POINTER(m, p, v)
#endif

}; // namespace
#endif  //_MULTIPING_TRACE_H