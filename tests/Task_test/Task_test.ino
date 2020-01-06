#include <AUnit.h>
//#include <AUnitVerbose.h>
using namespace aunit;

#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused with << or >> streaming operators
#include <PrintEx.h>

#include <MultiPing.h>

static StreamEx out(Serial);

class MockTask : public MultiPing::Task {
  public:
    MockTask(int id) : MultiPing::Task(id) {
    }

    int count = 0;
    unsigned long lastDispatch = 0;
    
    bool dispatch( unsigned long now ) {
      lastDispatch = now;
      return (count++ > 2);  
    }

    MultiPing::TaskList getFastQueue() {
      return MultiPing::Task::fastQueue;
    }
    MultiPing::TaskList getSlowQueue() {
      return MultiPing::Task::slowQueue;
    }
    MultiPing::TaskList getWaiting() {
      return MultiPing::Task::waiting;
    }

    unsigned long getWhen() { return this->whenEnqueued; }
    unsigned long getDelay() { return this->usecDelay; }
};

test(slowQueue) {
  MockTask  mock(0);

  assertTrue( mock.getSlowQueue().isEmpty() );

  mock.enqueueLong( 1000ul );
  //printf("%8lu %8lu %8lu\n", micros(), mock.getWhen(), mock.getDelay() );
  
  assertFalse( mock.getSlowQueue().isEmpty() );

  for (int i = 0; i < 10; i++) {
    MultiPing::Task::run();
    if (mock.lastDispatch != 0) {
      //printf("%8lu %d %8lu\n", micros(), i, mock.lastDispatch );
      assertTrue( mock.lastDispatch >= (mock.getWhen()+mock.getDelay()) ); 
      break;
    }
    delayMicroseconds(100);
  }
  assertTrue(mock.lastDispatch != 0);
  assertTrue(mock.getSlowQueue().isEmpty() );
}

test(fastQueue) {
  MockTask  mock(0);

  assertTrue( mock.getFastQueue().isEmpty() );

  mock.enqueueShort( 1000ul );
  //printf("%8lu %8lu %8lu\n", micros(), mock.getWhen(), mock.getDelay() );
  
  assertFalse( mock.getFastQueue().isEmpty() );

  for (int i = 0; i < 10; i++) {
    MultiPing::Task::run();
    if (mock.lastDispatch != 0) {
      //printf("%8lu %d %8lu\n", micros(), i, mock.lastDispatch );
      assertTrue( mock.lastDispatch >= (mock.getWhen()+mock.getDelay()) ); 
      break;
    }
    delayMicroseconds(100);
  }
  assertTrue(mock.lastDispatch != 0);
  assertTrue(mock.getFastQueue().isEmpty() );
}


test(waiting) {
  MockTask  mock(0);
  assertTrue( mock.getWaiting().size() == 0 );

  mock.waitEvent();

  assertFalse( mock.getWaiting().size() == 0 );

  MultiPing::Task::run();
  assertFalse( mock.getWaiting().size() == 0 );
  assertTrue(mock.count == 1);
  MultiPing::Task::run();
  assertFalse( mock.getWaiting().size() == 0 );
  assertTrue(mock.count == 2);
  MultiPing::Task::run();
  assertFalse( mock.getWaiting().size() == 0 );
  assertTrue(mock.count == 3);

  MultiPing::Task::run();
  assertTrue( mock.getWaiting().size() == 0 );
  assertTrue(mock.count == 4);
  
  
}

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
    TestRunner::run();
}
