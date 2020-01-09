#ifndef _MULTIPING_DEVICE_H_
#define _MULTIPING_DEVICE_H_ 1

#include <IGPIO.h>

namespace MultiPing {

class Device {
   public:
    Device() {}
    virtual void reset(const IGPIO& trigger, const IGPIO& echo) {
        trigger.low();
        echo.output();
        echo.low();
    }
    unsigned int usecWaitEchoLowTimeout = 10u;
    unsigned int usecTriggerPulseDuration = 24u;
    unsigned long usecMaxEchoStartDelay = 500u;
    unsigned long usecMaxEchoDuration = 60000ul;

   protected:
};


}
#endif // _MULTIPING_DEVICE_H_