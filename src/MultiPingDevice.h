#ifndef _MULTIPING_DEVICE_H_
#define _MULTIPING_DEVICE_H_ 1

#include <IGPIO.h>

namespace MultiPing {

class Device {
   public:
    Device() {}

    virtual void reset() {}
    unsigned int usecWaitEchoLowTimeout = 10u;
    unsigned int usecTriggerPulseDuration = 24u;
    unsigned long usecMaxEchoStartDelay = 500u;
    unsigned long usecMaxEchoDuration = 60000ul;

   protected:
};

template<BOARD::pin_t TPIN, BOARD::pin_t EPIN>
class DefaultDevice : public Device {
   public:
    DefaultDevice() {}

    virtual void reset() {
        trigger.low();
        echo.output();
        echo.low();
    }
    
    protected:
    GPIO<TPIN> trigger;
    GPIO<EPIN> echo;
};

}  // namespace MultiPing
#endif  // _MULTIPING_DEVICE_H_