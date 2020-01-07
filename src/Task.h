#ifndef _MULTIPING_TASK_H
#define _MULTIPING_TASK_H 1

#define _MULTIPING_DEBUG_ 0

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
#include <avr/pgmspace.h>
#endif

#if _MULTIPING_DEBUG_
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused with << or >> streaming operators
#include <PrintEx.h>
static StreamEx out(Serial);
#else
class StreamEx;
#endif

namespace MultiPing {
class Task;
class TaskList;

inline int compareUnsigned(unsigned long a, unsigned long b) {
    unsigned long d0 = a - b;
    // printf("d0 %8lu\n", d0);
    if (d0 == 0ul) return 0;
    unsigned long d1 = b - a;
    // printf("d1 %8lu\n", d1);
    return (d0 < d1) ? +1 : -1;
}

inline unsigned long lessThanUnsigned(unsigned long a, unsigned long b) {
    return compareUnsigned(a, b) == -1;
}

inline unsigned long unsignedDistance(unsigned long a, unsigned long b) {
    return (a - b < b - a) ? a - b : b - a;
}

class TaskList {
   public:
    class Iterator {
       public:
        Iterator(const Iterator& that);
        Iterator(TaskList* list, Task* task);
        Iterator& operator=(const Iterator& that);
        Task* operator*() { return current; };
        bool operator!=(const Iterator& that);
        bool operator==(const Iterator& that);
        Iterator& operator++();
#if 0
                    void dump() {
                        Serial.print("Iterator ");
                        Serial.print( (unsigned long) list, HEX );
                        Serial.print(" ");
                        Serial.print( (unsigned long)(list->head), HEX );
                        Serial.print(" ");
                        Serial.print( (unsigned long) current, HEX );
                        Serial.println();
                    }
#endif
       protected:
        TaskList* list;
        Task* current;
        friend class TaskList;
    };

    TaskList() : count(0), head(nullptr), tail(nullptr) {}

    ~TaskList() {}

    Iterator begin() {
        Iterator it(this, this->head);
        return it;
    }
    Iterator end() {
        Iterator it(this, nullptr);
        return it;
    }

    bool isEmpty() { return count == 0; }

    unsigned int size() { return count; }

    Task* peek();
    Task* last();
    Task* pop();
    bool contains(Task* task);

    Iterator insert(Iterator& before, Task* task);
    void push_back(Task* task);
    void push_front(Task* task);
    void push_priority(Task* task);

    void erase(Task* which);
    Iterator erase(Iterator which);

    // test support methods
    bool check(const char* tag = nullptr);
    void dump();

   protected:
    unsigned int count;
    Task* head;
    Task* tail;
};

class Task {
   public:
    Task(int id) : id(id), next(nullptr), prev(nullptr), waiting(false) {
        nextTask = all;  // link this Task to previously constructed head
        all = this;      // new head
        // Serial.print("Task("); Serial.print(id); Serial.print(") ");
        // Serial.print( (unsigned long) all, HEX); Serial.print(" ");
        // Serial.print( (unsigned long) this, HEX); Serial.print(" ");
        // Serial.println( (unsigned long) nextTask, HEX);
    }
    virtual ~Task() {}
    virtual int getId() { return id; }
    virtual bool dispatch(unsigned long now) = 0;

    void enqueueShort(unsigned long usecDelay);
    void enqueueLong(unsigned long usecDelay);
    void waitEvent(bool waitEvent);

    void dump(const char* tag);

    static TaskList& getLongQueue() { return slowQueue; }
    static bool lessThan(Task* lhs, Task* rhs);

    static void run();
    static void report();
    static void print(unsigned long now);
    static void setDebugOutput(StreamEx* dbg) { Task::dbg = dbg; }
    static StreamEx* dbg;

   protected:
    int id;
    Task* next;
    Task* prev;
    friend class TaskList;
    friend class TaskList::Iterator;
    Task* nextTask;  // simple linked list of all Task objects; based on static
                     // all below
    bool waiting;    // true if waiting for event
    unsigned long whenEnqueued;  // usec (from micros())
    unsigned long usecDelay;     // usec wait requested

    static bool ready(unsigned long now, Task* task) {
        return !lessThanUnsigned(now, task->whenEnqueued + task->usecDelay);
    }

    // only valid is ! ready()
    static unsigned int usecRemaining(unsigned long now, Task* task) {
        unsigned long dt = (task->whenEnqueued + task->usecDelay) - now;
        return (unsigned int)dt;
    }

    static unsigned long cycleCount;
    static Task* all;
    static TaskList fastQueue;
    static TaskList slowQueue;
};
}  // namespace MultiPing
#endif