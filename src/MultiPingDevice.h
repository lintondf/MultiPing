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

enum InputModes { PULLUP, OPEN_COLLECTOR };

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

}  // namespace MultiPing

#endif  // _MULTIPING_DEVICE_H_