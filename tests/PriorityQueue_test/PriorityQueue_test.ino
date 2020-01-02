#include <ArduinoSTL.h>

#include <PriorityQueue.h>
#include <PrintEx.h>

StreamEx out = Serial;

bool lessThanInt( int a, int b ) {
  return a < b;
}

PriorityQueue<int> pq(lessThanInt);

void show(const char* objective) {
  out.print("Expecting: ");
  out.println(objective);
  out.printf("  Size %d\n", pq.size() );
  out.printf("  Not-empty %d\n", pq.isEmpty() );
  out.printf("  peek %d\n", pq.peek() );
  out.printf("  last %d\n", pq.last() );
  out.print("  [");
  
  for (PriorityQueue<int>::iterator it = pq.begin(); it != pq.end(); it++) {
    out.print(*it);
    out.print(", ");
  }
  out.println("]");  
}
void setup() {
  Serial.begin(115200);
  while (! Serial)
  out.println();
  out.println("PriorityQueue_test");
  out.printf("Empty %d\n", pq.isEmpty() );
  pq.push(1);
  show("[1]");
  out.printf("pop: %d\n", pq.pop() );
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
  out.printf("pop: %d\n", pq.pop() );
  show("[1,2,2,3,4]");
  out.println("test complete");
}

void loop() {
}
