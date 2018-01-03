#include "synch.h"
#include "list.h"

extern List<int> buffer;
extern int full, empty;
extern Lock bufAcc, isEmptyLock, isFullLock;
extern Condition isEmpty, isFull;

class Producer {
   public:
      Producer(const char *debugName);
      ~Producer();
      
      static void produce(void *arg);
      const char* getName() { return name; }
   
   private:
      const char *name;
};
