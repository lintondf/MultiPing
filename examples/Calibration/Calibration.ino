#include <MultiPing.h>
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused
// with << or >> streaming operators
#include <PrintEx.h>
#include <RunningStatistics.h>
#include <math.h>

StreamEx out(Serial);

#define MULTIPING_TRACE true
#include <MultiPingTrace.h>

void calibrate( MultiPing::Device* device, float a) {
  Serial.print("Trigger: "); Serial.println( device->getTriggerPin() );
  Serial.print("Echo:    "); Serial.println( device->getEchoPin() );
  const int N = 1000;
  out.printf("Averaging %d samples...\n", N );
  RunningStatistics stats, stat2;
  int notIdle = 0;
  for (int i = 0; i < N; i++) {
    //      TRACE_TIMESTAMP(0,__LINE__);
    unsigned long start = micros();
    device->begin();
    while (device->isEchoing())
      notIdle++;
    if ((micros() - start) > 1000000ul) {
      out.println("Device stuck echoing");
      device->reset();
      return;
    }

    device->beginTrigger();
    delayMicroseconds(24);
    device->finishTrigger();
    unsigned long waitStart = micros();
    while (!device->isEchoing())
      if ((micros() - start) > 1000000ul) {
        out.println("Device never started echoing");
        device->reset();
        return;
      }
    unsigned long echoStart = micros();
    while (device->isEchoing())
      if ((micros() - start) > 1000000ul) {
        out.println("Device echo never ended");
        device->reset();
        return;
      }
    unsigned long echoFinish = micros();
    stats.push(MultiPing::unsignedDistance(echoStart, waitStart));
    stat2.push(MultiPing::unsignedDistance(waitStart, echoFinish));
    delay(50);
  }
  out.printf(
    "Trigger TE to Echo LE [us] (mean, std, min, max): %10.2f %10.3f "
    "%10.2f %10.2f\n",
    stats.mean(), sqrt(stats.variance()), stats.minimum(),
    stats.maximum());
  out.printf(
    "Echo LE to Echo TE [us] (mean, std, min, max):    %10.2f %10.3f "
    "%10.2f %10.2f\n",
    stat2.mean(), sqrt(stat2.variance()), stat2.minimum(),
    stat2.maximum());
  out.printf(
    "                   [in] (mean, std, min, max):    %10.2f %10.3f "
    "%10.2f %10.2f\n",
    0.5 * a * stat2.mean(), 0.5 * a * sqrt(stat2.variance()), 0.5 * a * stat2.minimum(),
    0.5 * a * stat2.maximum());
}

MultiPing::Device*  devices[6];

float a; // speed of sound [inches/microsecond]


const BOARD::pin_t  R0T = BOARD::D54; // PINF:0/A0
const BOARD::pin_t  R0E = BOARD::D55; //
const BOARD::pin_t  R1T = BOARD::D56; // PINF:2/A2
const BOARD::pin_t  R1E = BOARD::D57; //
const BOARD::pin_t  R2T = BOARD::D58; // PINF:4/A4
const BOARD::pin_t  R2E = BOARD::D59;
const BOARD::pin_t  L0T = BOARD::D22; // PINA:0
const BOARD::pin_t  L0E = BOARD::D23; 
const BOARD::pin_t  L1T = BOARD::D24; // PINA:2
const BOARD::pin_t  L1E = BOARD::D25;
const BOARD::pin_t  L2T = BOARD::D26; // PINA:4
const BOARD::pin_t  L2E = BOARD::D27;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("Sensor Calibration");
  MultiPing::Units::setTemperature( (int) 5.0 / 9.5 * (71.0 - 32.0) );
  a = MultiPing::Units::us2in(1ul);
  Serial.print( a ); Serial.println( " in/us" );

  //  device = new BaseTwoPinPullup<BOARD::D22, BOARD::D23>(); //TODO FAILS
  // device = new MultiPing::Default2PinDevice<BOARD::D54, BOARD::D55>();
  //  device = new MultiPing::Default2PinDevice<BOARD::D56, BOARD::D57>();  // N/C
  devices[0] = new MultiPing::Default2PinDevice<L0T, L0E, MultiPing::PULLUP>();
  devices[1] = new MultiPing::Default2PinDevice<L1T, L1E, MultiPing::PULLUP>();
  devices[2] = new MultiPing::Default2PinDevice<L2T, L2E, MultiPing::PULLUP>();
  devices[3] = new MultiPing::Default2PinDevice<R0T, R0E, MultiPing::PULLUP>();
  devices[4] = new MultiPing::Default2PinDevice<R1T, R1E, MultiPing::PULLUP>();
  devices[5] = new MultiPing::Default2PinDevice<R2T, R2E, MultiPing::PULLUP>();
//  devices[6] = new MultiPing::Default1PinDevice<BOARD::D14>();
//  devices[6] = new MultiPing::Default1PinDevice<BOARD::D14, MultiPing::OPEN_COLLECTOR>();

#if 0
  unsigned long start = micros();
  int n = 0;
  for (long i = 0; i < 1000l; i++) {
    n += (int) device->isEchoing();
  }
  Serial.println( micros() - start );
  start = micros();
  for (long i = 0; i < 10000l; i++) {
    n += (int) device->isEchoing();
  }
  Serial.println( micros() - start );
  start = micros();
  for (long i = 0; i < 100000l; i++) {
    n += (int) device->isEchoing();
  }
  Serial.println( micros() - start );

  /* No echo activity until after trigger goes low */
  unsigned long t[30];
  bool e[30];
  device->begin();
  device->beginTrigger();
  delayMicroseconds(24);
  device->finishTrigger();

  for (long k = 0; k < 30; k++) {
    t[k] = micros();
    e[k] = device->isEchoing();
  }
  for (long k = 0; k < 30; k++) {
    out.printf("%2d %10lu %d\n", k, t[k], (int) e[k] );
  }
#endif
}

void loop() {
  //TRACE_START( &out );
  calibrate(devices[0], a);
    calibrate(devices[1], a);
    calibrate(devices[2], a);
    calibrate(devices[3], a);
    calibrate(devices[4], a);
    calibrate(devices[5], a);
  //TRACE_DUMP
  //TRACE_DISABLE
  delay(500);
}
