#include <Arduino.h>
#include <MultiPingUnits.h>

#if MULTIPING_UNITS_SET_FLOAT_TERMPERATURE
#include <math.h>
#endif

namespace MultiPing {

Units::SoS_t Units::a = {330, 0};

#ifndef MULTIPING_UNITS_FIXED_SPEEDOFSOUND   
const Units::SoS_t Units::speedOfSound[nSoS] = {
    {312, 512, 639}, {315, 709, 633}, {318, 873, 627}, {322, 7, 621},   {325, 110, 615},
    {328, 185, 609}, {331, 230, 604}, {334, 248, 598}, {337, 239, 593}, {340, 203, 588},
    {343, 142, 583}, {346, 56, 578},  {348, 946, 573}, {351, 812, 569}, {354, 655, 564},
    {357, 475, 560}, {360, 273, 555},
};


#if MULTIPING_UNITS_SET_FLOAT_TERMPERATURE
void Units::setTemperature(float dC) {
    double a0 = sqrt(1.4f * 286.9f * (273.15f + dC));
    int mps = (int)trunc(a0);
    int mmps = (int)trunc(1000.0 * (a0 - mps));
    double da0 = 1.4f * 286.9f / (2.0 * a0);  // D[Sqrt[k (a + t)], t]
    Units::a.ms = mps;
    Units::a.mms = mmps;
    Units::a.dadT = 0;
}
#endif //MULTIPING_UNITS_SET_FLOAT_TERMPERATURE

void Units::setTemperature(int dC) {
    dC = (-30 > dC) ? -30 : dC;
    dC = (+50 < dC) ? +50 : dC;
    int iSoS = (dC + 30) / 5;
    Units::a = speedOfSound[iSoS];
    int dT = dC - (5 * iSoS - 30);
    int da = Units::a.dadT * dT;
    int dams = da / 1000;
    da -= 1000 * dams;
    Units::a.ms += dams;
    Units::a.mms += da;
}
#endif //MULTIPING_UNITS_FIXED_SPEEDOFSOUND


float Units::us2m(unsigned long us) {
    float a = (float)Units::a.ms + 0.001f * (float)Units::a.mms;
    return a * 1e-6 * (float)us;
}

float Units::us2cm(unsigned long us) { return 100.0 * us2m(us); }

float Units::us2ft(unsigned long us) { return us2m(us) / 0.3048; }

float Units::us2in(unsigned long us) { return 12.0 * us2ft(us); }

};  // namespace MultiPing