#ifndef MULTIPING_H
#define MULTIPING_H 1

/*********************************************************************
 * A library for handling multiple ultrasonic sensors via multitasking
 *********************************************************************/

namespace MultiPing {

    int compareUnsigned( unsigned long a, unsigned long b ) {
    int d0 = a - b;
    if (d0 == 0ul)
        return 0;
    int d1 = b - a;
    return (d1 < d0) ? +1 : -1;
    }

    unsigned long lessThanUnsigned( unsigned long a, unsigned long b ) {
    return compareUnsigned( a, b ) == -1;
    }


    class Task {
        public:
        protected:

    };

    class Device {
        public:
        protected:
    };

    class Sonar {
        public:
        protected:
    }; // Sonar
} // namespace MultiPing
#endif //MULTIPING_H