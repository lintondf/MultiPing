#include <AUnit.h>
//#include <AUnitVerbose.h>
using namespace aunit;

//#include <ArduinoSTL.h>

#include "Filter.h"

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only

  Filter f(0.8, 100.0 );
  for (float v = 0.0; v < 10.0; v++) {
    Serial.print(v);
    Serial.print(" ");
    Serial.println( f.add(v) );
  }
}


void loop() {
  TestRunner::run();
}
