#include <Task.h>


#if _MULTIPING_DEBUG_
#include <RunningStatistics.h>
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused
                                   // with << or >> streaming operators
#include <PrintEx.h>
#endif

using namespace MultiPing;

/*---TaskListIterator--------------------------------------------------------*/

TaskList::Iterator::Iterator(const TaskList::Iterator& that) {
    list = that.list;
    current = that.current;
}

TaskList::Iterator::Iterator(TaskList* list, Task* task) {
    this->list = list;
    this->current = task;
}

TaskList::Iterator& TaskList::Iterator::operator=(
    const TaskList::Iterator& that) {
    list = that.list;
    current = that.current;
    return (*this);
}

bool TaskList::Iterator::operator==(const TaskList::Iterator& that) {
    return current == that.current;
}

bool TaskList::Iterator::operator!=(const TaskList::Iterator& that) {
    return current != that.current;
}

TaskList::Iterator& TaskList::Iterator::operator++() {
    current = current->next;
    return (*this);
}

/*---TaskList----------------------------------------------------------------*/

bool TaskList::contains(Task* task) {
    Task* x = head;
    while (x != nullptr) {
        if (x == task) {
#if _MULTIPING_DEBUG_
            if (Task::dbg)
                Task::dbg->printf("%lx already in %lx\n", (unsigned long)task,
                                  (unsigned long)this);
            delay(1000);
#endif
            return true;
        }
        x = x->next;
    }
    return false;
}

bool TaskList::check(const char* tag) {
#if _MULTIPING_DEBUG_  // test support function; TODO move to test mock
    unsigned int bad = 0;
    if (count == 0) {
        if (head != nullptr)
            bad |= 0x00001;  // Task::dbg->printf("Empty; head not null\n");
        if (tail != nullptr)
            bad |= 0x00002;  // Task::dbg->printf("Empty; tail not null\n");
    } else if (count == 1) {
        if (head == nullptr || tail == nullptr) {
            bad |= 0x00004;  // Task::dbg->printf("one; head or tail null\n");
        } else {
            if (head != tail)
                0x00008;  // bad |= Task::dbg->printf("one; head NE tail\n");
            if (head->prev != nullptr)
                bad |=
                    0x000010;  // Task::dbg->printf("one; head.prev NE null\n");
            if (head->next != nullptr)
                bad |=
                    0x00020;  // Task::dbg->printf("one; head.next NE null\n");
        }
        if (head->prev == head || head->next == head)
            bad |= 0x00040;  // Task::dbg->printf("one; self reference\n");
    } else {
        if (head == nullptr || tail == nullptr) {
            bad |= 0x00080;  // Task::dbg->printf("many; head or tail null\n");
        } else {
            if (head == tail)
                bad |= 0x00100;  // Task::dbg->printf("many; head EQ tail\n");
            if (head->prev != nullptr)
                bad |=
                    0x00200;  // Task::dbg->printf("many; head.prev NE null\n");
            if (tail->next != nullptr)
                bad |=
                    0x00400;  // Task::dbg->printf("many; tail.next NE null\n");
            if (head->prev == head || head->next == head)
                bad |= 0x00800;  // Task::dbg->printf("many; head self
                                 // reference\n");
            if (tail->prev == tail || tail->next == tail)
                bad |= 0x01000;  // Task::dbg->printf("many; tail self
                                 // reference\n");
            int n = 2;
            Task* prev = head;
            Task* task = head->next;
            while (task != tail) {
                if (task->prev != prev)
                    bad |= 0x02000;  // Task::dbg->printf("many; %d bad prev\n",
                                     // n - 1);
                if (task->prev == task || task->next == task)
                    bad |= 0x04000;  // Task::dbg->printf("many; task self
                                     // reference\n");
                n++;
                prev = task;
                task = task->next;
            }
            if (n != count)
                bad |= 0x08000;  // Task::dbg->printf("many; count %d; n %d\n",
                                 // count, n);
        }
    }
    if (bad) {
        Task::dbg->printf("%x in %lx from %s\n", bad, (unsigned long)this,
                          (tag != nullptr) ? tag : "?");
        dump();
    }
    return bad == 0;
#else
    return true;
#endif
}

void TaskList::dump() {
#if _MULTIPING_DEBUG_
    if (Task::dbg) {
        Task::dbg->printf("TaskList %d h %8lx t %8lx\n", count,
                          (unsigned long)head, (unsigned long)tail);
        for (Iterator it = begin(); it != end(); it++) {
            // it.dump();
            (*it)->dump("  ");
        }
        delay(1000);
    }
#endif
}

Task* TaskList::peek() { return head; }

Task* TaskList::last() { return tail; }

Task* TaskList::pop() {
    Task* task = head;
    erase(task);
    task->next = task->prev = nullptr;
    return task;
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
TaskList::Iterator TaskList::insert(Iterator& before, Task* task) {
    if ((*before)->prev == nullptr) {
        push_front(task);
    } else if (before.current == nullptr) {
        push_back(task);
    } else {
        Task* prev_node = (*before)->prev;
        /* make next of new node as next of prev_node */
        task->next = prev_node->next;

        /* make the next of prev_node as new_node */
        prev_node->next = task;

        /* make prev_node as previous of new_node */
        task->prev = prev_node;

        /* change previous of new_node's next node */
        if (task->next != nullptr)
            task->next->prev = task;
        else
            tail = task;
        count++;
    }
    return Iterator(this, task);
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
void TaskList::push_front(Task* task) {
    count++;
    /* make next of new node as head and previous as NULL */
    task->next = head;
    task->prev = nullptr;

    /* change prev of head node to new node */
    if (head != nullptr) {
        head->prev = task;
        if (head == tail) head->next = nullptr;
    } else
        tail = task;
    /* move the head to point to the new node */
    head = task;
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
void TaskList::push_back(Task* task) {
    count++;
    Task* last = tail;

    /* this new node is going to be the last node, so
          make next of it as nullptr*/
    task->next = nullptr;

    /* if the Linked List is empty, then make the new
          node as head and tail */
    if (head == nullptr) {
        task->prev = nullptr;
        head = task;
        tail = task;
        return;
    }

    /* change the next of last node */
    tail->next = task;

    /* make last node as previous of new node */
    task->prev = last;
    tail = task;
}

void TaskList::push_priority(Task* task) {
    if (this->size() == 0 || Task::lessThan(task, peek())) {
        this->push_front(task);  // add at head
    } else {                     // add in middle
        for (TaskList::Iterator it = this->begin(); it != this->end(); it++) {
            if (Task::lessThan(task, *it)) {
                insert(it, task);
                return;
            }
        }
        this->push_back(task);  // add at tail
    }
}

TaskList::Iterator TaskList::erase(TaskList::Iterator which) {
    Task* current = which.current;
    which.current = which.current->next;
    erase(current);
    return which;
}

/**
 * Attribution:
 * https://www.geeksforgeeks.org/delete-a-node-in-a-doubly-linked-list/ [CC
 * BY-SA 4.0]
 */
void TaskList::erase(Task* current) {
    if (head == nullptr) return;
    if (current == nullptr) current = tail;

    count--;
    /* If node to be deleted is head node */
    if (head == current) head = current->next;
    /* If node to be deleted is tail node */
    if (tail == current) tail = current->prev;

    /* Change next only if node to be deleted is NOT the last node */
    if (current->next != nullptr) current->next->prev = current->prev;

    /* Change prev only if node to be deleted is NOT the first node */
    if (current->prev != nullptr) current->prev->next = current->next;
    current->next = nullptr;
    current->prev = nullptr;
}

/*---Task--------------------------------------------------------------------*/

Task* Task::all = nullptr;
TaskList Task::fastQueue;
TaskList Task::slowQueue;
StreamEx* Task::dbg;

unsigned long Task::cycleCount = 0ul;

void Task::report() {
#if _MULTIPING_DEBUG_ > 0
    Task::dbg->printf("Tasks: ");
    Task* task = all;
    while (task != nullptr) {
        Task::dbg->printf("%d; ", task->getId());
        task = task->nextTask;
    }
    Task::dbg->printf("\n");
    Task::dbg->printf("fast %lx; slow %lx\n", (unsigned long)&fastQueue,
                      (unsigned long)&slowQueue);
#endif                    
}

void Task::dump(const char* tag) {
#if _MULTIPING_DEBUG_ > 0
    if (Task::dbg) {
        Task::dbg->printf("%s  %8lx/%2d: p %8lx n %8lx; %8lu %8lu\n", tag,
                          (unsigned long)this, this->getId(),
                          (unsigned long)this->prev, (unsigned long)this->next,
                          this->whenEnqueued, this->usecDelay);
    }
#endif    
}

void Task::print(unsigned long now) {
#if _MULTIPING_DEBUG_ > 0
    for (TaskList::Iterator it = slowQueue.begin(); it != slowQueue.end();
         it++) {
        if (dbg)
            dbg->printf("%3d slowQueue %8lu %8lu (%8lu) %8lu %s\n",
                        (*it)->getId(), now, (*it)->whenEnqueued,
                        ((*it)->whenEnqueued + (*it)->usecDelay - now),
                        (*it)->usecDelay, (ready(now, (*it)) ? "T" : "F"));
    }
    for (TaskList::Iterator it = fastQueue.begin(); it != fastQueue.end();
         it++) {
        if (dbg)
            dbg->printf("%3d fastQueue %8lu %8lu (%8lu) %8lu %s\n",
                        (*it)->getId(), now, (*it)->whenEnqueued,
                        ((*it)->whenEnqueued + (*it)->usecDelay - now),
                        (*it)->usecDelay, (ready(now, (*it)) ? "T" : "F"));
    }
#endif
}

void Task::run() {
    unsigned long now = micros();
#if _MULTIPING_DEBUG_ > 0
    if (cycleCount == 0L) {
        print(now);
    }
#endif
    cycleCount++;
#if _MULTIPING_DEBUG_ >= 3    
    if ((slowQueue.size() > 0 || fastQueue.size() > 0))
        if (dbg)
            dbg->printf("run %8lu %8lu %u:%d/%u:%d\n", now, cycleCount,
                        slowQueue.size(), slowQueue.isEmpty(), fastQueue.size(),
                        fastQueue.isEmpty());
#endif                    
    if (!slowQueue.isEmpty()) {
        // if the top is not ready neither is anyone else
        if (ready(now, slowQueue.peek())) {
            now = micros();
            Task* task = slowQueue.pop();
#if _MULTIPING_DEBUG_ >= 2
            if (dbg)
                dbg->printf("slow %8lu/%8lu: %d\n", now, cycleCount,
                            task->getId());
#endif                                
            task->dispatch(now);
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
        Task* task = fastQueue.pop();
#if _MULTIPING_DEBUG_ >= 2
            if (dbg)
                dbg->printf("fast %8lu/%8lu: %d\n", now, cycleCount,
                            task->getId());
#endif                            
        task->dispatch(now);
    }

    // scan all sensors for any waiting pin events; once per cycle
    Task* task = all;
    while (task != nullptr) {
        if (task->waiting) {
            now = micros();
#if _MULTIPING_DEBUG_ >= 2
                if (dbg)
                    dbg->printf("wait %8lu/%8lu: %d\n", now, cycleCount,
                                task->getId());
#endif                                
            task->dispatch(now);
        }
        task = task->nextTask;
    }
}

bool Task::lessThan(Task* lhs, Task* rhs) {
    bool result = lessThanUnsigned(lhs->whenEnqueued + lhs->usecDelay,
                                   rhs->whenEnqueued + rhs->usecDelay);
    return result;
}

void Task::enqueueShort(unsigned long usecDelay) {
    this->whenEnqueued = micros();
    this->usecDelay = usecDelay;
    fastQueue.push_priority(this);
}

void Task::enqueueLong(unsigned long usecDelay) {
    this->whenEnqueued = micros();
    this->usecDelay = usecDelay;
    slowQueue.push_priority(this);
}

void Task::waitEvent(bool waitEvent) { this->waiting = waitEvent; }

/*===========================================================================*/