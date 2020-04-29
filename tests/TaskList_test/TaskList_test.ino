#include <AUnit.h>
//#include <AUnitVerbose.h>
using namespace aunit;

#include <MultiPingTask.h>
#undef __GXX_EXPERIMENTAL_CXX0X__  // otherwise GPIO::SFR bit shifts confused with << or >> streaming operators
#include <PrintEx.h>
static StreamEx out(Serial);

using namespace MultiPing;

class MockTask : public Task {
  public:
    MockTask(int id) : Task(id) {
      whenEnqueued = 0ul;
      usecDelay = id;
    }
    ~MockTask() {}

    int count = 0;
    unsigned long lastDispatch = 0;
    
    bool dispatch( unsigned long now ) {
      lastDispatch = now;
      return (count++ > 2);  
    }

    TaskList getFastQueue() {
      return fastQueue;
    }
    TaskList getSlowQueue() {
      return slowQueue;
    }

    unsigned long getWhen() { return this->whenEnqueued; }
    unsigned long getDelay() { return this->usecDelay; }
};


static MockTask* mockTasks[5]/* = {
  new MockTask(0),
  new MockTask(1),
  new MockTask(2),
  new MockTask(3),
  new MockTask(4),
}*/;
static MockTask* mockTask10;
static MockTask* mockTask20;
static MockTask* mockTask30;

#define COMPARE(count, empty, head, tail, expected )                               \
  assertEqual( pq.size(), (unsigned int) count );                 \
  assertEqual( pq.isEmpty(), empty );                                              \
  if (! pq.isEmpty() ) {                                                           \
    assertEqual( pq.peek()->getId(), head );                                                \
    assertEqual( pq.last()->getId(), tail );                                                \
    int i = 0;                                                                     \
    for (TaskList::Iterator it = pq.begin(); it != pq.end(); it++) {     \
      assertEqual( expected[i++], (*it)->getId() );                                           \
    }                                                                              \
  }                                                                               

void print( TaskList list ) {
  Serial.print("TL: [");
    for (TaskList::Iterator it = list.begin(); it != list.end(); it++) { 
      Serial.print((*it)->getId()); Serial.print(",");
    }
   Serial.println("]");
}

test(allList) {
  TaskList list;  
  assertTrue( list.isEmpty() );
  list.push_front( mockTasks[1] );
  assertFalse( list.isEmpty() );
  assertTrue( list.size() == 1u );
  assertEqual( list.peek()->getId(), 1 );
  assertEqual( list.last()->getId(), 1 );
  TaskList::Iterator it = list.begin();
  assertTrue( (*it)->getId() == 1 );
  assertTrue( it != list.end() );
  it++;
  assertTrue( it == list.end() );
  assertEqual( list.pop()->getId(), 1 );
  assertTrue( list.isEmpty() );

  list.push_back( mockTasks[1] );
  assertFalse( list.isEmpty() );
  assertTrue( list.size() == 1u );
  assertEqual( list.peek()->getId(), 1 );
  assertEqual( list.last()->getId(), 1 );
  it = list.begin();
  assertTrue( (*it)->getId() == 1 );
  assertTrue( it != list.end() );
  it++;
  assertTrue( it == list.end() );
  assertEqual( list.pop()->getId(), 1 );
  assertTrue( list.isEmpty() );

  assertTrue( list.check() );
  for (int i = 0; i < 5; i++) {
    list.push_back( mockTasks[i] );
  }
  for (int i = 0; i < 5; i++) {
    assertEqual( list.pop()->getId(), i );
  }
  assertTrue( list.isEmpty() );
  assertTrue( list.check() );
  for (int i = 0; i < 5; i++) {
    list.push_front( mockTasks[i] );
    assertTrue( list.check() );
  }
  for (int i = 4; i >= 0; i--) {
    assertEqual( list.pop()->getId(), i );
    assertTrue( list.check() );
  }
  assertTrue( list.isEmpty() );

  for (int i = 0; i < 5; i++) {
    list.push_back( mockTasks[i] );
    assertTrue( list.check() );
  }
  it = list.begin();
  for (int i = 0; i < 5; i++) {
    assertEqual( (*it)->getId(), i );
    it++;
  }
  assertTrue( it == list.end() ); 
  assertTrue( list.check() );

  it = list.begin();
  it = list.insert( it, mockTask10 );
  assertTrue( list.check() );
  assertEqual( list.peek()->getId(), 10 );
  assertEqual( (*it)->getId(), 10 );
  it++;
  it++;
  assertEqual( (*it)->getId(), 1 );
  it = list.insert( it, mockTask20 );
  assertTrue( list.check() );
  assertEqual( (*it)->getId(), 20 );
  it++;
  it++;
  it++;
  it++;
  it = list.insert( it, mockTask30 );
  assertTrue( list.check() );
  assertEqual( (*it)->getId(), 30 );
  it++; // tail node
  assertEqual( list.last()->getId(), 4);
  list.erase( it );
  assertEqual( list.last()->getId(), 30);
  assertEqual( list.peek()->getId(), 10 );
  list.erase( list.begin() );
  assertEqual( list.peek()->getId(), 0 );
  assertEqual( (*(list.begin()++))->getId(), 20 );
  assertTrue( list.check() );
  it = list.erase( list.begin()++ );
  assertTrue( list.check() );
  assertEqual( (*it)->getId(), 1 );
  assertEqual( list.last()->getId(), 30);
  list.erase( list.end() );
  assertTrue( list.check() );
  assertEqual( list.last()->getId(), 3);
  assertTrue( list.size() == 4 );
  it = list.begin();
  it++;
  it++;
  list.erase(it);
  assertTrue( list.check() );
  it = list.begin();
  it++;
  list.erase(it);
  assertTrue( list.check() );
  list.erase(list.end());
  assertTrue( list.check() );
  list.erase(list.begin());
  assertTrue( list.check() );

}

#if 1
test(emptyPriorityQueue) {
  TaskList pq;
  assertTrue( pq.check() );
  assertTrue( pq.isEmpty() );
}

test(onePriorityQueue) {
  TaskList pq;
  pq.push_priority( mockTasks[1] );
  assertTrue( pq.check() );
  int expected[] = {1};
  COMPARE( 1, false, 1, 1, expected );
  assertEqual( pq.pop()->getId(), 1 );
  assertTrue( pq.check() );
  assertTrue( pq.isEmpty() );
  Task* t1 = mockTasks[1];
  Task* t2 = mockTasks[2];
  assertTrue(Task::lessThan( t1, t2 ));
  assertFalse(Task::lessThan( t1, t1 ));
  assertFalse(Task::lessThan( t2, t1 ));
}

test(fullPriorityQueue) {
  TaskList pq;
  pq.push_priority(mockTasks[1]);
  pq.push_priority(mockTasks[2]);
  assertTrue( pq.check() );
  int expected1[] = {1,2};
  COMPARE( 2, false, 1, 2, expected1 );
  pq.push_priority(mockTasks[3]);
  assertTrue( pq.check() );
  int expected2[] = {1,2,3};
//  print(pq);
  COMPARE( 3, false, 1, 3, expected2 );
  pq.push_priority(mockTasks[0] );  // new head
  assertTrue( pq.check() );
  int expected3[] = {0,1,2,3};
  COMPARE( 4, false, 0, 3, expected3 );
  pq.push_priority(mockTasks[4]);  // new tail
  assertTrue( pq.check() );
  int expected4[] = {0,1,2,3,4};
  COMPARE( 5, false, 0, 4, expected4 );
  pq.push_priority(new MockTask(2));  // dup in middles 
  assertTrue( pq.check() );
  int expected5[] = {0,1,2,2,3,4};
  COMPARE( 6, false, 0, 4, expected5 );
  assertEqual(pq.pop()->getId(), 0);
  assertTrue( pq.check() );
  int expected6[] = {1,2,2,3,4};
  COMPARE( 5, false, 1, 4, expected6 );  
  assertEqual(pq.pop()->getId(), 1);
  assertTrue( pq.check() );
  int expected7[] = {2,2,3,4};
  COMPARE( 4, false, 2, 4, expected7 );  
  assertEqual(pq.pop()->getId(), 2);
  assertTrue( pq.check() );
  int expected8[] = {2,3,4};
  COMPARE( 3, false, 2, 4, expected8 );  
  assertEqual(pq.pop()->getId(), 2);
  assertTrue( pq.check() );
  int expected9[] = {3,4};
  COMPARE( 2, false, 3, 4, expected9 );  
  assertEqual(pq.pop()->getId(), 3);
  assertTrue( pq.check() );
  int expected10[] = {4};
  COMPARE( 1, false, 4, 4, expected10 );  
  assertEqual(pq.pop()->getId(), 4);
  assertTrue( pq.check() );
  assertTrue( pq.isEmpty() );
}
#endif

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
  Serial.println("TaskList_test");
  MultiPing::Task::setDebugOutput( &out );
  for (int i = 0; i < 5; i++) {
    MockTask* m = new MockTask(i);
    mockTasks[i] = m;
  }
  mockTask10 = new MockTask(10);
  mockTask20 = new MockTask(20);
  mockTask30 = new MockTask(30);
}

void loop() {
  TestRunner::run();
}
