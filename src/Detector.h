#ifndef _DETECTOR_H
#define _DETECTOR_H 1

#include <MultiPing.h>
#include <Filter.h>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
static int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

/*
 * DO NOT STATICALLY ALLOCATE OBJECTS OF THIS CLASS.
 */
class Detector : public MultiPing::Sonar, public MultiPing::Sonar::Handler {
public:
    bool  offline;

    enum Sides {
        FRONT =   0,
        LEFT  = 10,
        RIGHT = 20,
        REAR  = 30,
        TRAILER_LEFT  = 40,
        TRAILER_RIGHT = 50,
        TRAILER_REAR  = 60
    };
    Detector(int id, MultiPing::Device* device) : 
        MultiPing::Sonar( id, device ),
        MultiPing::Sonar::Handler(),
        filter( 0.5, device->usecMaxEchoDuration ),
        offline(false),
        mmLastDetect(0) {
        for (int i = 0; i < MultiPing::Sonar::N_ERRORS; i++)
          errors[i] = 0;
    }

    ~Detector() {
    }

    void start(unsigned long startAt, unsigned long repeatCycle) {
        Sonar::start(startAt, repeatCycle, this);
    }

    void event( MultiPing::Sonar* sensor, long usecOneWay ) {
#if _MULTIPING_DEBUG_ == 0
      //Serial.print(sensor->getId()); Serial.print(": ");
#endif   
      if (usecOneWay < 0) { // error case
          errors[(int)-usecOneWay]++;
#if _MULTIPING_DEBUG_      
        MultiPing::Task::dbg->printf("%2d: ERROR %ld\n", sensor->getId(), usecOneWay);
#else
        //Serial.println(usecOneWay);
#endif        
      } else {
        if (usecOneWay == 0) {
          // Serial.print(getId());
          // Serial.print(" ");
          // Serial.print( millis() );
          // Serial.println(" NO_PING; ");
          usecOneWay = sensor->device->usecMaxEchoDuration / 2;
          errors[0]++;
        }
        // Serial.println(usecOneWay);
        unsigned long mm = MultiPing::Units::us2mm(usecOneWay);
        mmLastDetect = (long) filter.add(mm); // smoothed
        mmLastDetect = mm; // TODO test only
#if _MULTIPING_DEBUG_ > 1
        MultiPing::Task::dbg->printf("%2d: %8ld %6ld %5d\n", sensor->getId(), mm, mmLastDetect, freeMemory() );
#else
       // Serial.print(mmLastDetect); Serial.print(" " );
       // Serial.println(mm);
#endif
      }
    }

    /**
     * @returns last detection distance in millimeters
     */
    long getLastDetect() {
      return mmLastDetect;
    }

    void getAndClearErrors( int counts[MultiPing::Sonar::N_ERRORS]) {
      for (int i = 0; i < MultiPing::Sonar::N_ERRORS; i++) {
        counts[i] = errors[i];
        errors[i] = 0;
      }
    }

protected:
    int                 errors[MultiPing::Sonar::N_ERRORS];
    long                mmLastDetect;
    Filter              filter;
};

#endif //_DETECTOR_H