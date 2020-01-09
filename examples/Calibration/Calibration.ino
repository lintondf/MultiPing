#include <MultiPing.h>
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused
                                   // with << or >> streaming operators
#include <PrintEx.h>
#include <RunningStatistics.h>

StreamEx out(Serial);

void calibrate( MultiPing::Device* device) {
    RunningStatistics stats, stat2;
    for (int i = 0; i < 100; i++) {
      device->begin();
      while (device->isEchoing())
          ;
      
      device->beginTrigger();
      delayMicroseconds(24);
      device->finishTrigger();
      unsigned long waitStart = micros();
      while (!device->isEchoing())
          ;
      unsigned long echoStart = micros();
      while (device->isEchoing())
          ;
      unsigned long echoFinish = micros();
      stats.push(MultiPing::unsignedDistance(echoStart, waitStart));
      stat2.push(MultiPing::unsignedDistance(waitStart, echoFinish));
      delay(50);
    }
    out.printf(
            "Trigger TE to Echo LE (us, mean, var, min, max): %10.0f "
            "%10.0f %10.0f %10.0f\n",
            stats.mean(), stats.variance(), stats.minimum(),
            stats.maximum());
    out.printf(
            "Echo LE to Echo TE (us, mean, var, min, max):    %10.0f "
            "%10.0f %10.0f %10.0f\n",
            stat2.mean(), stat2.variance(), stat2.minimum(),
            stat2.maximum());      
}

MultiPing::Device*  device;

void setup() {
  Serial.begin(115200);
  while(!Serial) ;
  device = new MultiPing::Default2PinDevice<BOARD::D14, BOARD::D15>();
}

void loop() {
  calibrate(device);
}
