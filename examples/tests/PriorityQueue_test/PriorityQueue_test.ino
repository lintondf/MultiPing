#include <AUnit.h>
//#include <AUnitVerbose.h>
using namespace aunit;

#include <ArduinoSTL.h>

#include <PriorityQueue.h>

bool lessThanInt( int a, int b ) {
  return a < b;
}

#define COMPARE(count, empty, head, tail, expected )                               \
  assertEqual( pq.size(), (PriorityQueue<int>::size_type) count );                 \
  assertEqual( pq.isEmpty(), empty );                                              \
  if (! pq.isEmpty() ) {                                                           \
    assertEqual( pq.peek(), head );                                                \
    assertEqual( pq.last(), tail );                                                \
    int i = 0;                                                                     \
    for (PriorityQueue<int>::iterator it = pq.begin(); it != pq.end(); it++) {     \
      assertEqual( expected[i++], *it );                                           \
    }                                                                              \
  }                                                                               

test(empty) {
  PriorityQueue<int> pq(lessThanInt);

  assertTrue( pq.isEmpty() );
}

test(one) {
  PriorityQueue<int> pq(lessThanInt);
  pq.push(1);
  int expected[] = {1};
  COMPARE( 1, false, 1, 1, expected );
  assertEqual( pq.pop(), 1 );
  assertTrue( pq.isEmpty() );
}

test(full) {
  PriorityQueue<int> pq(lessThanInt);
  pq.push(1);
  pq.push(2);
  int expected1[] = {1,2};
  COMPARE( 2, false, 1, 2, expected1 );
  pq.push(3);
  int expected2[] = {1,2,3};
  COMPARE( 3, false, 1, 3, expected2 );
  pq.push(0);  // new head
  int expected3[] = {0,1,2,3};
  COMPARE( 4, false, 0, 3, expected3 );
  pq.push(4);  // new tail
  int expected4[] = {0,1,2,3,4};
  COMPARE( 5, false, 0, 4, expected4 );
  pq.push(2);  // dup in middles 
  int expected5[] = {0,1,2,2,3,4};
  COMPARE( 6, false, 0, 4, expected5 );
  printf("pop: %d\n", pq.pop() );
  int expected6[] = {1,2,2,3,4};
  COMPARE( 5, false, 1, 4, expected6 );  
}


void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
#if 0

void setup() {
  Serial.begin(115200);
  while (! Serial)
  printf("\nPriorityQueue_test\n");
#if 0  
  printf("Empty %d\n", pq.isEmpty() );
  show("[1]");
  printf("pop: %d\n", pq.pop() );
  show("[]");
  pq.push(1);
  pq.push(2);
  show("[1,2]");
  pq.push(3);
  show("[1,2,3]");
  pq.push(0);  // new head
  show("[0,1,2,3]");
  pq.push(4);  // new tail
  show("[0,1,2,3,4]");
  pq.push(2);  // dup in middles 
  show("[0,1,2,2,3,4]");
  printf("pop: %d\n", pq.pop() );
  show("[1,2,2,3,4]");
  printf("test complete\n");
#endif  
}

void loop() {
  TestRunner::run();
}
#endif
