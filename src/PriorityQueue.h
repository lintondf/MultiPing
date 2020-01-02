// PriorityQueue.h
#ifndef _PRIORITY_QUEUE_H
#define _PRIORITY_QUEUE_H 1

#include <ArduinoSTL.h>
#include <list>


template <class T>
class PriorityQueue : public std::list<T> {
    public:

        typedef bool (*LessThanFunction)(T, T);
        PriorityQueue(LessThanFunction lessThan) : std::list<T>(), lessThan(lessThan) {

        }
        bool inline isEmpty() { 
            return this->size() == 0;
        }

        T inline peek() {
            return this->front();
        }

        T inline last() {
            return this->back();
        }

        void push( T a ) {
            if (this->size() == 0 || lessThan(a, front())) {
                this->push_front(a); // add at head
            } else { // add in middle
                for (typename PriorityQueue<T>::iterator it = this->begin(); it != this->end(); it++) {
                    if (lessThan(a, *it)) {
                        insert(it, a);
                        return;
                    }
                }
                this->push_back(a);  // add at tail
            }
        }

        T inline pop() {
            T head = front();
            pop_front();
            return head;
        }

    protected:
        LessThanFunction lessThan;
};
#endif // _PRIORITY_QUEUE_H