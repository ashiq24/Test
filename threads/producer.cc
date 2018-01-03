#include "producer.h"
#include "unistd.h"

#define MAX_BUFFER_SIZE 15
List<int> buffer;
int full = 0, empty = MAX_BUFFER_SIZE;
Lock bufAcc("Buffer Access Lock"), isEmptyLock("Lock for Buffer Empty"), isFullLock("Lock for Buffer Full");
Condition isEmpty("Buffer is Empty", &isEmptyLock), isFull("Buffer is Full", &isFullLock);

Producer::Producer(const char *debugName) {
   name = debugName;
}

Producer::~Producer() {}

void Producer::produce(void *arg) {
   const char *name = ((Producer *)arg)->name;
   while(true) {
      bufAcc.Acquire();
      if(!empty) {
         bufAcc.Release();
         isFullLock.Acquire();
         isFull.Wait();
         isFullLock.Release();
      } else {
         int x = 5;
         printf("Producer %s created %d\n", name, x);
         buffer.Append(x);
         full++;
         empty--;
         sleep(1);
         bufAcc.Release();
         isEmptyLock.Acquire();
         isEmpty.Signal();
         isEmptyLock.Release();
      }
   }
}
