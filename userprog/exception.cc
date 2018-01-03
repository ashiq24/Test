// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "processTable.h"
#include "thread.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void PCForward()
{
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc += 4;
	machine->WriteRegister(NextPCReg, pc);
}

void Dummy(void* arg)
{
	(currentThread->space)->InitRegisters();
	(currentThread->space)->RestoreState();
	machine->Run();

	return;
}

void SysCallHaltHandler();
void SysCallExecHandler();
void SysCallExitHandler();
void SysCallReadHandler();
void SysCallWriteHandler();


void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    printf("Inside ExceptionHandler\n");
	switch (which)
	{
		case SyscallException:
		    printf("Inside SyscallException\n");
			switch (type)
			{
				case SC_Halt:
					SysCallHaltHandler();
					break;

				case SC_Exec:
					SysCallExecHandler();
					break;

				case SC_Exit:
					SysCallExitHandler();
					break;

				case SC_Read:
					SysCallReadHandler();
					break;

				case SC_Write:
				default:
					break;
			}
			break;

		case PageFaultException:
			DEBUG('e', "Page Fault Exception\n");
			PCForward();
			break;

		case ReadOnlyException:
			DEBUG('e', "Read Onle Exception\n");
			PCForward();
			break;

		case BusErrorException:
			DEBUG('e', "Bus Error Exception\n");
			PCForward();
			break;

		case OverflowException:
			DEBUG('e', "Overflow Exception\n");
			PCForward();
			break;

		case IllegalInstrException:
			DEBUG('e', "Illegal Instruction Exception\n");
			PCForward();
			break;

		case NumExceptionTypes:
			DEBUG('e', "Num Exception Types\n");
			PCForward();
			break;

		default:
			printf("Unexpected user mode exception %d %d\n", which, type);
			break;
	}
}

void
SysCallHaltHandler()
{
	DEBUG('a', "Shutdown, initiated by user program.\n");
	interrupt->Halt();
	PCForward();

	return;
}


void
SysCallExecHandler()
{
	char fileName[100];
	int arg = machine->ReadRegister(4);
	int i = 0;

	do
	{
		machine->ReadMem(arg + i, 1, (int*)&fileName[i]);

	} while(fileName[i++] != '\0');


	OpenFile* executable = fileSystem->Open(fileName);
	if (executable == NULL)
    {
        printf("Unable to open file %s\n", fileName);
		machine->WriteRegister(2, 0);
		PCForward();
        return;
    }

	Thread* thread = new Thread(fileName);
	AddrSpace* space = new AddrSpace(executable);
	thread->space=space;
	int ret = pTable->Alloc((void*)thread);
	delete executable;

	if(ret == -1)
	{
		printf("Allocation failed\n" );
		PCForward();
		return;
	}

	thread->pid = ret;
	machine->WriteRegister(2, ret);
	thread->Fork(Dummy, NULL);
	PCForward();

	return;
}

void
SysCallExitHandler()
{
	int arg = machine->ReadRegister(4);
	int index = currentThread->pid;

	pTable->Release(index);
	currentThread->space->ReleaseMemory();
	currentThread->Finish();

	PCForward();

	return;
}

void
SysCallReadHandler()
{
	printf("here now\n");
	unsigned int addr = machine->ReadRegister(4);
	unsigned int size = machine->ReadRegister(5);
	//unsigned int id = machine->ReadRegister(6);

	char* buffer = new char[size];

	for(int i = 0; i < size; i++)
	{
		_readAvail->P();
		buffer[i] = _console->GetChar();
	}
	buffer[size] = '\0';
	printf("%s\n",buffer );
	for(int i = 0; i < size;i++)
	{
		machine->WriteMem(addr,1, (int)buffer[i]);
		addr++;
	}

	DEBUG('z', "Size of string = %d\n", size);
	machine->WriteRegister(2,size);
	bzero(buffer, sizeof(char) * size);

	PCForward();

	return;
}


void
SysCallWriteHandler()
{
	unsigned int addr = machine->ReadRegister(4);
	unsigned int size = machine->ReadRegister(5);

	char* buffer = new char[size];
	for(int i = 0; i < size; i++)
	{
		int c;
		machine->ReadMem(addr, 1, &c);
		buffer[i] = (char)c;
	}

	for(int i = 0; i < size; i++)
	{
		_console->PutChar(buffer[i]);
		_writeDone->P();
	}

	PCForward();

	return;
}
