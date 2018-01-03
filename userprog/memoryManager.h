#include "synch.h"
#include "bitmap.h"



class MemoryManager
{
private:
    BitMap *bitMap;
	Lock* memoryLock;

public:
    MemoryManager(int nPages)
    {
        bitMap = new BitMap(nPages);
		memoryLock = new Lock("memLock");
    }

    int AllocPage()
    {
        memoryLock->Acquire();
        int page = bitMap->Find();
        memoryLock->Release();
        return page;
    }

    void FreePage(int physPageNumber)
    {
        memoryLock->Acquire();
        bitMap->Clear(physPageNumber);
        memoryLock->Release();

		return;
    }

    bool PageIsAllocated(int physPageNum)
    {
        memoryLock->Acquire();
        bool ret =  bitMap->Test(physPageNum);
        memoryLock->Release();

		return ret;
    }

    int GetAvailableMemory()
    {
		memoryLock->Acquire();
        int ret = bitMap->NumClear();
		memoryLock->Release();

		return ret;
    }

};

extern MemoryManager* memoryManager;
//extern Lock* memoryLock;
