#include "synch.h"
#include "list.h"

extern List<int> buffer;
extern int full, empty;
extern Lock bufAcc, isEmptyLock, isFullLock;
extern Condition isEmpty, isFull;

class Consumer {
   public:
      Consumer(const char *debugName);
      ~Consumer();
      
      static void consume(void *arg);
      const char* getName() { return name; }
   
   private:
      const char *name;
};
