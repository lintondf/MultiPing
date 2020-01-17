#ifndef _AJ_SR04_M_H
#define _AJ_SR04_M_H 1

#include <MultiPingDevice.h>

namespace MultiPing {

/*********************************************************************
 * AJ-SR04M ultrasonic sensor device-level manager
 * MODE 1 - Trigger/Echo
 *********************************************************************/
template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
class AJ_SR04_M : public GenericDevice<TPIN, EPIN, MODE> {
   public:
    AJ_SR04_M() : GenericDevice<TPIN, EPIN, MODE>() {
        this->usecWaitEchoLowTimeout = 4;
        this->usecTriggerPulseDuration = 20;
        this->usecMaxEchoStartDelay = 250;
        this->usecMaxEchoDuration = 60000ul;
    }
};

template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
using AJ_SR04_M_2Pin = AJ_SR04_M<TPIN, EPIN, MODE>;
template <BOARD::pin_t PIN, InputModes MODE = OPEN_COLLECTOR>
using AJ_SR04_M_1Pin = AJ_SR04_M<PIN, PIN, MODE>;

}  // namespace MultiPing
#endif  //_AJ_SR04_M_H