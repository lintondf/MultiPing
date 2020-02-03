#include <PrintEx.h>
#include <MultiPingTrace.h>

StreamEx out(Serial);
using namespace MultiPing;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  delay(500);
  Serial.println("Trace_example");
  TRACE_START(&out)
  TRACE_STOPAFTERDUMP
  TRACE_TIMESTAMP(0,1)
  TRACE_VALUE(0,10,1234)
  TRACE_POINTER(0,20,&out)
  TRACE_TIMESTAMP(0,2)
//  TRACE_DUMP
//  TRACE_STOP
}

void loop() {
  TRACE_TIMESTAMP(0,1)
  TRACE_VALUE(0,10,1234)
  TRACE_POINTER(0,20,&out)
  TRACE_TIMESTAMP(0,2)
}
