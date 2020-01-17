#ifndef _HC_SR04_H
#define _HC_SR04_H 1

#include <MultiPingDevice.h>

namespace MultiPing {

/*********************************************************************
 * HC-SR04 ultrasonic sensor device-level manager
 *********************************************************************/
template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
class HC_SR04 : public GenericDevice<TPIN, EPIN, MODE> {
   public:
    HC_SR04() : GenericDevice<TPIN, EPIN, MODE>() {
        this->usecWaitEchoLowTimeout = 4;
        this->usecTriggerPulseDuration = 10;
        this->usecMaxEchoStartDelay = 200;
        this->usecMaxEchoDuration =
            38000ul;  // https://www.elecrow.com/download/HC_SR04%20Datasheet.pdf
    }
};

template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
using HC_SR04_2Pin = HC_SR04<TPIN, EPIN, MODE>;
template <BOARD::pin_t PIN, InputModes MODE = OPEN_COLLECTOR>
using HC_SR04_1Pin = HC_SR04<PIN, PIN, MODE>;

}  // namespace MultiPing
#endif  //_HC_SR04_H