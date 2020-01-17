#ifndef _MULTIPING_DEVICE_H_
#define _MULTIPING_DEVICE_H_ 1

/***
 * MultiPing - Arduino Ultrasonic Sensor Library
 * (C) Copyright 2019 - Blue Lightning Development, LLC.
 * D. F. Linton. support@BlueLightningDevelopment.com
 *
 * SPDX-License-Identifier: MIT
 * See separate LICENSE file for full text
 */

#include <IGPIO.h>

namespace MultiPing {

/*********************************************************************
 * Ultrasonic sensor device-level manager
 * Pure virtual base class
 *********************************************************************/
class Device {
   public:
    Device() {}
    ~Device() {}

    /** 
     * Get the trigger pin assignment
     * @return pin on BOARD
     */
    virtual BOARD::pin_t getTriggerPin() const = 0;

    /** 
     * Get the echo pin assignment
     * @return pin on BOARD
     */
    virtual BOARD::pin_t getEchoPin() const = 0;

    /**
     * Test the echoing status of the device
     * @return true if echoing
     */
    virtual bool isEchoing() const = 0;

    /**
     * Perform any device-specific setup at the start of a ping cycle
     */
    virtual void begin() const = 0;

    /**
     * Perform any device-specific action required to start trigger a ping
     */
    virtual void beginTrigger() const = 0;

    /**
     * Perform any device-specific action required to finish trigger a ping
     */
    virtual void finishTrigger() const = 0;

    /**
     * Perform any device-specific action required after the end of an echo is
     * detected/
     */
    virtual void finish() const = 0;

    /**
     * Perform any device-specific action required to reset a potentially
     * malfunctioning sensor
     */
    virtual void reset() const = 0;

    /// minimum microseconds to wait for any prior echo activity to finish
    unsigned int usecWaitEchoLowTimeout = 10u;
    /// minimum microseconds between beginTrigger() and finishTrigger() actions
    unsigned int usecTriggerPulseDuration = 24u;
    /// maximum microseonds after finishTrigger() to wait for an echo to start
    unsigned long usecMaxEchoStartDelay = 500ul;
    /// maximum microseonds to wait for a started echo to finish; NO_ECHO
    /// warning on timeout
    unsigned long usecMaxEchoDuration = 60000ul;
};

enum InputModes {
  PULLUP, OPEN_COLLECTOR
};



/*********************************************************************
 * Generic two-wire ultrasonic sensor device-level manager
 *********************************************************************/
template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
class GenericDevice : public Device {
   public:
    GenericDevice() {}

    inline BOARD::pin_t getTriggerPin() const { return TPIN; }
    inline BOARD::pin_t getEchoPin() const { return EPIN; }

    bool isEchoing() const { return echo.read(); }

    void begin() const {
        echo.input();
        if (MODE == PULLUP) {
          echo.pullup();
        }
    }

    void beginTrigger() const {
        trigger.output();
        trigger.low();
        trigger.high();
    }

    void finishTrigger() const {
        trigger.low();
        if (TPIN == EPIN) {
          echo.input();
        }
    }

    void finish() const {}

    void reset() const {
        trigger.low();
        echo.output();
        echo.low();
    }

   protected:
    GPIO<TPIN> trigger;
    GPIO<EPIN> echo;
};

template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
using Default2PinDevice = GenericDevice<TPIN, EPIN, MODE>;
template <BOARD::pin_t PIN, InputModes MODE = OPEN_COLLECTOR>
using Default1PinDevice = GenericDevice<PIN, PIN, MODE>;

#if 0
template <template <BOARD::pin_t TPIN, BOARD::pin_t EPIN> class Device, BOARD::pin_t PIN>
class OnePinWithPullupWrapper : public Device<PIN, PIN> {
    public:
    OnePinWithPullupWrapper() : Device<PIN, PIN>() {
    }
    void finishTrigger() const {
        Device<PIN,PIN>::finishTrigger();
        Device<PIN,PIN>::echo.input();
        Device<PIN,PIN>::echo.pullup();
    }
};

enum InputModes {
  PULLUP, OPEN_COLLECTOR
};

class SBase {
  public:
  SBase(BOARD::pin_t TPIN, BOARD::pin_t EPIN) {
    Serial.print("SBase(");
    Serial.print(TPIN);
    Serial.print(",");
    Serial.print(EPIN);
    Serial.println(")");
  }
  void input() {
    Serial.println("SBase::input");
  };
  void listen() {
    Serial.println("SBase::listen");
  };
};

template <BOARD::pin_t TPIN, InputModes = OPEN_COLLECTOR>
class O;

template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes = OPEN_COLLECTOR>
class S;

template<BOARD::pin_t TPIN, BOARD::pin_t EPIN>
class S<TPIN, EPIN, PULLUP> : public SBase {
  public:
  S() : SBase(TPIN, EPIN) {}
  void input() {
    SBase::input();
    Serial.println("PULLUP");
  }
};

template<BOARD::pin_t TPIN, BOARD::pin_t EPIN>
class S<TPIN, EPIN, OPEN_COLLECTOR> : public SBase {
  public:
  S() : SBase(TPIN, EPIN) {}
  void input() {
    SBase::input();
    Serial.println("OPEN_COLLECTOR");
  }
};

template<BOARD::pin_t PIN>
class O<PIN, PULLUP> : public SBase {
  public:
  O() : SBase(PIN, PIN) {}
  void input() {
    SBase::input();
    Serial.println("PULLUP");
  }
  void listen() {
    SBase::listen();
    Serial.println("SWITCH");
  }
};

template<BOARD::pin_t PIN>
class O<PIN, OPEN_COLLECTOR> : public SBase {
  public:
  O() : SBase(PIN, PIN) {}
  void input() {
    SBase::input();
    Serial.println("OPEN_COLLECTOR");
  }
  void listen() {
    SBase::listen();
    Serial.println("SWITCH");
  }
};


/*
  delay(500);
  S<BOARD::D14, BOARD::D15, PULLUP> pullup;
  S<BOARD::D14, BOARD::D15, OPEN_COLLECTOR> open;
  S<BOARD::D14, BOARD::D15> which;
  pullup.input();
  pullup.listen();
  open.input();
  open.listen();
  which.input();
  which.listen();
  O<BOARD::D16, PULLUP> pullup1;
  O<BOARD::D16, OPEN_COLLECTOR> open1;
  O<BOARD::D16> which1;
  pullup1.input();
  pullup1.listen();
  open1.input();
  open1.listen();
  which1.input();
  which1.listen();
  delay(500);
  while(true) ;
*/
#endif 

}  // namespace MultiPing

#endif  // _MULTIPING_DEVICE_H_