#ifndef _MULTIPING_DEVICE_H_
#define _MULTIPING_DEVICE_H_ 1

#include <IGPIO.h>

namespace MultiPing {

class Device {
   public:
    Device() {}

    virtual bool isEchoing() = 0;
    virtual void begin() = 0;
    virtual void beginTrigger() = 0;
    virtual void finishTrigger() = 0;
    virtual void finish() = 0;
    virtual void reset() = 0;
    unsigned int usecWaitEchoLowTimeout = 10u;
    unsigned int usecTriggerPulseDuration = 24u;
    unsigned long usecMaxEchoStartDelay = 500u;
    unsigned long usecMaxEchoDuration = 60000ul;
};

template<BOARD::pin_t PIN>
class Default1PinDevice : public Device {
   public:
    DefaultDevice() {}

    bool isEchoing() {
        return combined.read();
    }

    void begin() {
        combined.input();
        combined.pullup();
    }

    void beginTrigger() {
        combined.output();
        combined.low();
        combined.high(); 
    }

    void finishTrigger() {
        combined.low();
        combined.input();
        combined.pullup();
    }

    void finish() {
    }

    void reset() {
        combined.low();
    }
    
    protected:
    GPIO<PIN> combined;
};

template<BOARD::pin_t TPIN, BOARD::pin_t EPIN>
class Default2PinDevice : public Device {
   public:
    DefaultDevice() {}

    bool isEchoing() {
        return echo.read();
    }

    void begin() {
        echo.input();
        echo.pullup();
    }

    void beginTrigger() {
        trigger.output();
        trigger.low();
        trigger.high(); 
    }

    void finishTrigger() {
        trigger.low();
#if ONE_PIN_ENABLED == true
    if (trigger.getPin() == echo.getPin()) {
        echo.input();
#if INPUT_PULLUP == true
        echo.pullup();
#endif
        }
#endif
    }

    void finish() {
    }

    void reset() {
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