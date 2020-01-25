#include <AUnit.h>
//#include <AUnitVerbose.h>
using namespace aunit;

#include <MultiPing.h>

#ifdef MULTIPING_UNITS_FIXED_SPEEDOFSOUND
test(fixed) {
  int A[] = MULTIPING_UNITS_FIXED_SPEEDOFSOUND;
  unsigned long a = 1000ul * (unsigned long)A[0];
  a += (unsigned long)A[1];
  unsigned long t = 1000000ul;
  assertEqual( t, MultiPing::Units::s2us(1) );
  assertEqual( MultiPing::Units::us2mm(t), a );  
}
#else
test(basic) {
  unsigned long t = 1000000ul;
  MultiPing::Units::setTemperature(0);
  assertEqual( t, MultiPing::Units::s2us(1) );
  assertEqual( MultiPing::Units::us2mm(t), 331230ul );
  assertEqual( MultiPing::Units::us2m(t), 331.230f );
  assertNear( MultiPing::Units::us2cm(t), 33123.0f, 1e-2 );
  assertNear( MultiPing::Units::us2ft(t), 1086.71f, 1e-2 );
  assertNear( MultiPing::Units::us2in(t), 13040.55f, 1e-2 );
}

test(tabulatedPoints) {
  unsigned long t = 1000000ul;
  
  int T[] = {-35, -30, -25, -20, -15, -10, -5,  0, 5, 10,  15,  20,  25,  30,  35,  40,  45,  50,  55};
  unsigned long A[] = {312512, 312512,  315709,  318873,  322007,  325110,  328185,  331230,  334248,  337239,  340203,  343142,  346056,  348946,  351812,  354655,  357475,  360273,  360273};
  int N = sizeof(T) / sizeof(T[0]);
  for (int i = 0; i < N; i++) {
    MultiPing::Units::setTemperature(T[i]);
    assertEqual( MultiPing::Units::us2mm(t), A[i] );
    assertTrue( abs(MultiPing::Units::us2m(t) - 0.001f* (float) A[i]) < 1e-3 );    
  }
}

test(interpolatedPoints) {
  for (float t = -30.0; t <= 50.0; t += 1.0) {
    MultiPing::Units::setTemperature((int) t);
    double a0 = sqrt(1.4f*286.9f*(273.15f+t));
    float a1 = MultiPing::Units::us2m(1000000ul);
//    Serial.print(t); Serial.print(" ");
//    Serial.print(a0); Serial.print(" ");
//    Serial.print(a1); Serial.print(" ");
//    Serial.println( 1e6*(a1 - a0) );
    assertTrue( abs(a1-a0) < 0.005 );
  }  
}

#if MULTIPING_UNITS_SET_FLOAT_TERMPERATURE
test(floatingTemperatures) {
  for (float t = -30.0; t <= 50.0; t += 0.1) {
    MultiPing::Units::setTemperature(t);
    double a0 = sqrt(1.4f*286.9f*(273.15f+t));
    float a1 = MultiPing::Units::us2m(1000000ul);
//    Serial.print(t); Serial.print(" ");
//    Serial.print(a0); Serial.print(" ");
//    Serial.print(a1); Serial.print(" ");
//    Serial.println( 1e6*(a1 - a0) );
    assertTrue( abs(a1-a0) < 0.005 );
  }  
}
#endif
#endif

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
