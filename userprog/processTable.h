#include "synch.h"
#include "bitmap.h"

class ProcessTable
{
public:
    void** arr;
    int nProcesses;
	int tableSize;
	Lock* pTableLock;

    ProcessTable(int size)
    {
        arr = new void*[size+1];

        for(int i = 1; i <= size; i++) arr[i] = NULL;

        nProcesses = 0;
		tableSize = size;

		pTableLock = new Lock("pTableLock");
    }

    int Alloc(void* object)
    {
		pTableLock->Acquire();
		int ret = -1;

        for(int i = 1; i <= tableSize; i++)
        {
            if(arr[i] == NULL)
            {
                arr[i] = object;
                nProcesses++;
                ret = i;
				break;
            }
        }
		pTableLock->Release();
		return ret;

    }

    void* Get(int index)
    {
		pTableLock->Acquire();

        if(arr[index] != NULL)
        {
			pTableLock->Release();
            return arr[index];
        }

		pTableLock->Release();
        return NULL;
    }

    void Release(int index)
    {
        pTableLock->Acquire();
		arr[index] = NULL;
        nProcesses--;
		pTableLock->Release();

		return;
    }
};

extern ProcessTable* pTable;
