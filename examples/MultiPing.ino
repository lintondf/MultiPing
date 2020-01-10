#if 0
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused with << or >> streaming operators
#include <PrintEx.h>
static StreamEx out(Serial);
#endif

#if 1
#include <Detector.h>
#else
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
#endif

#include <MultiPing.h>
#include <UiLED.h>

UI::UiLED*  led;

//#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
//const IGPIO& L0T = VGPIO<BOARD::D54>();
//const IGPIO& L0E = VGPIO<BOARD::D55>();
//const IGPIO& L1T = VGPIO<BOARD::D56>();
//const IGPIO& L1E = VGPIO<BOARD::D57>();
//const IGPIO& L2T = VGPIO<BOARD::D58>();
//const IGPIO& L2E = VGPIO<BOARD::D59>();
//
//const IGPIO& R0T = VGPIO<BOARD::D10>();
//const IGPIO& R0E = VGPIO<BOARD::D11>();
//const IGPIO& R1T = VGPIO<BOARD::D32>();
//const IGPIO& R1E = VGPIO<BOARD::D33>();
//const IGPIO& R2T = VGPIO<BOARD::D34>();
//const IGPIO& R2E = VGPIO<BOARD::D35>();
//#else
//const IGPIO& L0T = VGPIO<BOARD::D14>();
//const IGPIO& L0E = VGPIO<BOARD::D15>();
//const IGPIO& L1T = VGPIO<BOARD::D16>();
//const IGPIO& L1E = VGPIO<BOARD::D17>();
//const IGPIO& L2T = VGPIO<BOARD::D18>();
//const IGPIO& L2E = VGPIO<BOARD::D19>();
//
//const IGPIO& R0T = VGPIO<BOARD::D2>();
//const IGPIO& R0E = VGPIO<BOARD::D3>();
//const IGPIO& R1T = VGPIO<BOARD::D4>();
//const IGPIO& R1E = VGPIO<BOARD::D5>();
//const IGPIO& R2T = VGPIO<BOARD::D9>();
//const IGPIO& R2E = VGPIO<BOARD::D10>();
//#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //MEGA
const BOARD::pin_t  L0T = BOARD::D54;
const BOARD::pin_t  L0E = BOARD::D55;
const BOARD::pin_t  L1T = BOARD::D56;
const BOARD::pin_t  L1E = BOARD::D57;
const BOARD::pin_t  L2T = BOARD::D58;
const BOARD::pin_t  L2E = BOARD::D59;
const BOARD::pin_t  R0T = BOARD::D30;
const BOARD::pin_t  R0E = BOARD::D31;
const BOARD::pin_t  R1T = BOARD::D32;
const BOARD::pin_t  R1E = BOARD::D33;
const BOARD::pin_t  R2T = BOARD::D34;
const BOARD::pin_t  R2E = BOARD::D35;
#else  // UNO, NANO
const BOARD::pin_t  L0T = BOARD::D14;
const BOARD::pin_t  L0E = BOARD::D15;
const BOARD::pin_t  L1T = BOARD::D16;
const BOARD::pin_t  L1E = BOARD::D17;
const BOARD::pin_t  L2T = BOARD::D18;
const BOARD::pin_t  L2E = BOARD::D19;
const BOARD::pin_t  R0T = BOARD::D2;
const BOARD::pin_t  R0E = BOARD::D3;
const BOARD::pin_t  R1T = BOARD::D4;
const BOARD::pin_t  R1E = BOARD::D5;
const BOARD::pin_t  R2T = BOARD::D9;
const BOARD::pin_t  R2E = BOARD::D10;
#endif

#define SENSORS 0
#if SENSORS
MultiPing::Sonar sensors[] = {
 MultiPing::Sonar( 10, d14, d15 ),
 MultiPing::Sonar( 20, d16, d17 ),
 MultiPing::Sonar( 30, d18, d19 ),
 MultiPing::Sonar( 10, d2, d3 ),
 MultiPing::Sonar( 20, d4, d5 ),
 MultiPing::Sonar( 30, d9, d10 ),
};
#else
static Detector* detectors[6];
#endif
#define COUNT(A) (sizeof(A)/sizeof(A[0]))
int nDetectors = COUNT(detectors);

//class Detection : public MultiPing::Sonar::Handler {
//  void event( MultiPing::Sonar* sensor, long usecOneWay ) {
//    if (usecOneWay == 0)
//      usecOneWay = sensor->device->usecMaxEchoDuration;
//    Serial.print( sensor->getId() ); Serial.print("/");
//    Serial.print( (unsigned long) sensor, HEX ); Serial.print(": ");
//    Serial.println( usecOneWay );
////   while(usecOneWay <= 0) ;
//  }
//};

uint8_t cksum(const uint8_t *addr, uint8_t len) {
  uint8_t cs = 0;
  while (len--) {
    cs ^= *addr++;
  }
  return cs;
}

class Monitor : public MultiPing::Task {
  public:
    static unsigned long interval;
    Monitor() : Task(0), led(nullptr) {
    }
  bool dispatch( unsigned long now ) {
    //1@12: ttttttttt,n;
    //n@:11   dddd,z,e,e;
    //1@5:    mmmm;
    //CRC$\n
    char line[1+12+6*11+1*5+3+1];
    int n = sprintf( line, "$%lu,%d;", now, nDetectors );
    // Serial.print( now ); Serial.print(" ");
    UI::Proximity proximities[nDetectors];
    for (int i = 0; i < nDetectors; i++) {
      n += sprintf( line+n, "%u", detectors[i]->getLastDetect() );
      //Serial.print( detectors[i]->getLastDetect() ); Serial.print("/");
      proximities[i] = UI::CLEAR;
      if (detectors[i]->getLastDetect() < 4000)
        proximities[i] = UI::WARNING;
      if (detectors[i]->getLastDetect() < 2500)
        proximities[i] = UI::HAZARD;
      if (detectors[i]->getLastDetect() <  300)
        proximities[i] = UI::DANGER;
      if (detectors[i]->getLastDetect() == 0)
        proximities[i] = UI::ERROR;
       
      int errors[MultiPing::Sonar::N_ERRORS];
      detectors[i]->getAndClearErrors(errors);
      for (int j = 0; j < MultiPing::Sonar::N_ERRORS; j++) {
        n += sprintf( line+n, ",%u", errors[j] );
        //Serial.print( errors[j] ); Serial.print(",");
      }
      line[n++] = ';';
    }
    if(led) led->update(proximities);
    //Serial.println( freeMemory() );
    n += sprintf( line+n, "%u;", freeMemory() );
    uint8_t crc = cksum( line, n );
    sprintf( line+n, "%2.2X$\n", crc );
    Serial.print(line);
    enqueueLong( interval );
    return false;
  }

  void setUI(UI::UiLED*  led) {
    this->led = led;
  }

  UI::UiLED*  led;
};

unsigned long Monitor::interval = MultiPing::Units::ms2us(180*2);


Monitor monitor;

void setup() {
  Serial.begin(115200);
  while(!Serial) ;
#if _MULTIPING_DEBUG_  
  MultiPing::Task::setDebugOutput( &out );
#endif  
  Serial.println("MultiPing");
  MultiPing::Task::report();
  led = new UI::UiLED();
  led->start();  
  monitor.setUI(led);
  monitor.enqueueLong( Monitor::interval );
//  int i = 0;
//  sensors[i].calibrate();
//  sensors[i].start(MultiPing::Units::ms2us(100) + MultiPing::Units::ms2us(40)*(long)i, MultiPing::Units::ms2us(1200), new Detection());
//  sensors[0].calibrate();
//  sensors[1].calibrate();
//  sensors[2].calibrate();
#if SENSORS
  for (int i = 0; i < COUNT(sensors); i++) 
    sensors[i].start(MultiPing::Units::ms2us(100) + MultiPing::Units::ms2us(40)*(long)i, MultiPing::Units::ms2us(1200), new Detection());
#else
  int i = 0;
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::LEFT  + 0, new MultiPing::Default2PinDevice<L0T, L0E>() );
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::LEFT  + 1, new MultiPing::Default2PinDevice<L1T, L1E>() );
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::LEFT  + 2, new MultiPing::Default2PinDevice<L2T, L2E>() );
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::RIGHT + 0, new MultiPing::Default2PinDevice<R0T, R0E>() );
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::RIGHT + 1, new MultiPing::Default2PinDevice<R1T, R1E>() );
  if (i < nDetectors) detectors[i++] = new Detector( Detector::Sides::RIGHT + 2, new MultiPing::Default2PinDevice<R2T, R2E>() );
  
  for (int i = 0; i < nDetectors; i++)
    detectors[i]->start();  
#endif    
  MultiPing::Task::print(micros());
}

void loop() {
  MultiPing::Task::run();
}
