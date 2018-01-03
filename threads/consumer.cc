#include "consumer.h"
#include "unistd.h"

Consumer::Consumer(const char *debugName) {
   name = debugName;
}

Consumer::~Consumer() {}

void Consumer::consume(void *arg) {
   const char *name = ((Consumer *)arg)->name;
   while(true) {
      bufAcc.Acquire();
      if(!full) {
         bufAcc.Release();
         isEmptyLock.Acquire();
         isEmpty.Wait();
         isEmptyLock.Release();
      } else {
         int x = buffer.Remove();
         printf("Consumer %s consumed %d\n", name, x);
         full--;
         empty++;
         sleep(1);
         bufAcc.Release();
         isFullLock.Acquire();
         isFull.Signal();
         isFullLock.Release();
      }
   }
}
