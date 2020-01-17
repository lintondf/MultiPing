#ifndef _JSN_SR04_T_H
#define _JSN_SR04_T_H 1

#include <MultiPingDevice.h>

namespace MultiPing {

/*********************************************************************
 * JSN-SR04T ultrasonic sensor device-level manager
 * MODE 1
 *********************************************************************/
template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
class JSN_SR04_T : public GenericDevice<TPIN, EPIN, MODE> {
   public:
    JSN_SR04_T() : GenericDevice<TPIN, EPIN, MODE>() {
        this->usecWaitEchoLowTimeout = 4;
        this->usecTriggerPulseDuration = 16;  //  10 us per datasheet
        this->usecMaxEchoStartDelay = 200;    // 150 us per datasheet
        this->usecMaxEchoDuration = 60000ul;  // file:///C:/Users/NOOK/Downloads/JSN-SR04T-2.0.pdf
    }
};

template <BOARD::pin_t TPIN, BOARD::pin_t EPIN, InputModes MODE = OPEN_COLLECTOR>
using JSN_SR04_T_2Pin = JSN_SR04_T<TPIN, EPIN, MODE>;
template <BOARD::pin_t PIN, InputModes MODE = OPEN_COLLECTOR>
using JSN_SR04_T_1Pin = JSN_SR04_T<PIN, PIN, MODE>;

}  // namespace MultiPing
#endif  //_JSN_SR04_T_H