#include <Task.h>

#define DEBUG 1 
#if DEBUG
#include <RunningStatistics.h>
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

TaskList::Iterator& TaskList::Iterator::operator=(const TaskList::Iterator& that) {
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

TaskList::Iterator&  TaskList::Iterator::operator++() {
    current = current->next;
    return (*this);
}


/*---TaskList----------------------------------------------------------------*/

Task* TaskList::peek() {
    return head;
}

Task* TaskList::last() {
    return tail;
}

Task* TaskList::pop() {
    Task* task = head;
    if (head != nullptr) {
        head = head->next;
        count--;
    }
    if (count == 0)
        tail = nullptr;
    return task;
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
TaskList::Iterator TaskList::insert( Iterator& before, Task* task) {
    if ((*before)->prev == nullptr) {
        push_front( task );
    } else if (before.current == nullptr) {
        push_back( task );
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
    return Iterator( this, task );
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
void TaskList::push_front( Task* task) {
    count++;
    /* make next of new node as head and previous as NULL */
    task->next = head; 
    task->prev = nullptr; 
  
    /* change prev of head node to new node */
    if (head != nullptr) 
        head->prev = task; 
    else
        tail = task;
    /* move the head to point to the new node */
    head = task;
#if 0    
    Serial.print( (unsigned long) task, HEX ); Serial.print(" ");
    Serial.print( (unsigned long) head, HEX ); Serial.print(" ");
    Serial.print( (unsigned long) tail, HEX ); Serial.print(" ");
    Serial.print( (unsigned long) (task->prev), HEX ); Serial.print(" ");
    Serial.print( (unsigned long) (task->next), HEX ); Serial.print(" ");
    Serial.println( count );
#endif    
}

/**
 * Attribution: https://www.geeksforgeeks.org/doubly-linked-list/ [CC BY-SA 4.0]
 */
void TaskList::push_back( Task* task) {
    count++;
    Task* last = tail;
  
    /* this new node is going to be the last node, so 
          make next of it as nullptr*/
    task->next = nullptr; 
  
    /* if the Linked List is empty, then make the new 
          node as head */
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

void TaskList::push_priority( Task* task) {
    //!!Serial.println("push "); Serial.print( this->size() ); 
    if (this->size() == 0 || Task::lessThan(task, peek())) {
        //!!Serial.println("at head");
        //!!if (this->size() > 0) {
            //!!Serial.print(task->whenEnqueued); Serial.print("/");
            //!!Serial.print(task->usecDelay); Serial.print(" ");
            //!!Serial.print(peek()->whenEnqueued); Serial.print("/");
            //!!Serial.print(peek()->usecDelay); Serial.print(" ");
            //!!Serial.println();
        //!!}
        this->push_front(task); // add at head
    } else { // add in middle
        //!!Serial.println("not head");
        for (TaskList::Iterator it = this->begin(); it != this->end(); it++) {
            //!!Serial.print(task->whenEnqueued); Serial.print("/");
            //!!Serial.print(task->usecDelay); Serial.print(" ");
            //!!Serial.print((*it)->whenEnqueued); Serial.print("/");
            //!!Serial.print((*it)->usecDelay); Serial.print(" ");
            //!!Serial.println();
            if (Task::lessThan(task, *it)) {
                insert(it, task);
                return;
            }
        }
        //!!Serial.println("tail");
        this->push_back(task);  // add at tail
    }
}

/**
 * Attribution: https://www.geeksforgeeks.org/delete-a-node-in-a-doubly-linked-list/ [CC BY-SA 4.0]
 */
TaskList::Iterator TaskList::erase( TaskList::Iterator which) {
    /* base case */
    if (head == nullptr) 
        return;  
    Task* current = which.current;
    if (current == nullptr)
        current = tail;

    count--;
    TaskList::Iterator it( this, current->next );

    /* If node to be deleted is head node */
    if (head == current)  
        head = current->next; 
    if (tail == current)
        tail = current->prev;
  
    /* Change next only if node to be deleted is NOT the last node */
    if (current->next != nullptr)  
        current->next->prev = current->prev;  
  
    /* Change prev only if node to be deleted is NOT the first node */
    if (current->prev != nullptr)  
        current->prev->next = current->next; 
    
    return it;
}


/*---Task--------------------------------------------------------------------*/

TaskList  Task::waiting;
TaskList  Task::fastQueue;
TaskList  Task::slowQueue;

unsigned long        Task::cycleCount = 0ul;

void Task::print(unsigned long now) {
#if DEBUG > 0
    for (TaskList::Iterator it = slowQueue.begin(); it != slowQueue.end(); it++) {
        printf("%3d slowQueue %8lu %8lu (%8lu) %8lu %s\n", (*it)->getId(), now, (*it)->whenEnqueued, (now - (*it)->whenEnqueued), (*it)->usecDelay,
            (ready(now, (*it)) ? "T" : "F") );        
    }
    for (TaskList::Iterator it = fastQueue.begin(); it != fastQueue.end(); it++) {
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
    //if (DEBUG > 3) printf("run %8lu %8lu %u:%d/%d:%d/%d\n", now, cycleCount, waiting.size(), 
    //    slowQueue.size(), slowQueue.isEmpty(), fastQueue.size(), fastQueue.isEmpty() );
    if (!slowQueue.isEmpty()) {
        // if the top is not ready neither is anyone else
        if (ready(now, slowQueue.peek())) {
            now = micros();
            if (DEBUG >= 2) printf("slow %8lu/%8lu: %d:%d\n", now, cycleCount, slowQueue.size(), fastQueue.size() );
            slowQueue.pop()->dispatch(now);
        }
    }
    for (TaskList::Iterator it = waiting.begin(); it != waiting.end(); ) {
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

bool Task::lessThan( Task* lhs, Task* rhs) {
    //printf("lessThan %p %2d %8lu %8lu\n", lhs, lhs->getId(), lhs->whenEnqueued, lhs->usecDelay );
    //printf("         %p %2d %8lu %8lu\n", rhs, rhs->getId(), rhs->whenEnqueued, rhs->usecDelay );
    bool result = lessThanUnsigned( lhs->whenEnqueued+lhs->usecDelay, rhs->whenEnqueued+rhs->usecDelay );
    //!!Serial.print(lhs->whenEnqueued+lhs->usecDelay); Serial.print(" < ");
    //!!Serial.print(rhs->whenEnqueued+rhs->usecDelay); Serial.print(" = ");
    //!!Serial.println((result) ? "T":"F");
    //printf("         %s\n", (result) ? "T":"F");
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
    //printf("enqueue %p %8lu %8lu\n", this, this->whenEnqueued, this->usecDelay );
    slowQueue.push_priority(this);
}

void Task::waitEvent() {
    waiting.push_back(this);
}


/*===========================================================================*/