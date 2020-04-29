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

#include <MemoryFree.h>

namespace MultiPing {
StreamEx* Trace::out = nullptr;
void (*Trace::stopAfterDump)(void) = nullptr;
int Trace::lowMemory = 0;
Trace::TraceBlock* Trace::head = nullptr;
Trace::TraceBlock* Trace::tail = nullptr;
Trace::States Trace::state = Trace::MULTIPING_TRACE_STOPPED;

int Trace::freeMemory() { return ::freeMemory(); }

void Trace::append(uint8_t major, uint8_t minor, ValueKinds kind, uint32_t value) {
    if (state != MULTIPING_TRACE_STARTED) return;
    if (freeMemory() < lowMemory) {
        trace_dump();  // flush and release blocks
    }
    if (head == nullptr) {
        head = new TraceBlock();
        tail = head;
    }
    if (tail == nullptr) {
        trace_dump();  // try to flush and release blocks
        return;        // out of memory
    }
    TraceEntry* current = &tail->entries[tail->n++];
    if (tail->n >= ENTRIES_PER_BLOCK) {
        TraceBlock* empty = new TraceBlock();
        tail->next = empty;
        tail = empty;
    }
    current->major = major;
    current->minor = minor;
    current->kind = kind;
    current->value = value;
}

void Trace::trace_dump() {
    if (state != MULTIPING_TRACE_STARTED) return;

    TraceBlock* block = head;
    out->printf("TRACE_DUMP > %d\n", freeMemory());
    uint32_t lastTime;
    while (block != nullptr) {
        for (unsigned i = 0; i < block->n; i++) {
            switch (block->entries[i].kind) {
                case TIMESTAMP:
                    lastTime = block->entries[i].value;
                    MULTIPING_TRACE_SERIAL->printf("T %10lu %3u %3u\n", lastTime,
                                                   block->entries[i].major,
                                                   block->entries[i].minor);
                    break;
                case VALUE:
                    MULTIPING_TRACE_SERIAL->printf("V %10lu %3u %3u %10lu\n", lastTime,
                                                   block->entries[i].major, block->entries[i].minor,
                                                   block->entries[i].value);
                    break;
                case POINTER:
                    MULTIPING_TRACE_SERIAL->printf("P %10lu %3u %3u %10lx\n", lastTime,
                                                   block->entries[i].major, block->entries[i].minor,
                                                   block->entries[i].value);
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
    out->printf("TRACE_DUMP < %d\n", freeMemory());
    if (stopAfterDump != nullptr) {
        (*stopAfterDump)();
    }
};

};  // namespace MultiPing