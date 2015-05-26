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
#include "fdtable.h"
#include "addrspace.h"

#include "mmu.h"


#include <string>
#include <sstream>
#include <vector>


int getArg(int num);
bool readString(int virtAddr,std::string& str);
bool readBuffer(int virtAddr, int size, char* buf);
bool writeBuffer(char* buf, int size, int virtAddr);
int tokenize(std::string str, char **&tokens);
void deleteTokens(int argc, char **tokens);
void SyscallHandler(int type);
bool ReadMem(int addr, int size, int *value);
bool WriteMem(int addr, int size, int value);

struct Args{
  int argc;
  char** argv;
  int userSpaceArgv;
};

OpenFileId syscallOpen(char *name);

void deleteArgs(int argc, char **tokens){
  for (int i = 0; i < argc; i++)
    delete[] tokens[i];
  delete[] tokens;
}

void threadRun(int argPtr){
  currentThread->space->InitRegisters();		// set the initial register values
  currentThread->space->RestoreState();		// load page table register

  currentThread->space->swap = new Swap((int)currentThread,
					currentThread->space->numPages);

  Args* userSpaceArgs = (Args*) argPtr;

  currentThread->space->copyArguments(userSpaceArgs->argc,
				      userSpaceArgs->argv,
				      userSpaceArgs->userSpaceArgv);

  machine->WriteRegister(4,userSpaceArgs->argc);
  machine->WriteRegister(5,userSpaceArgs->userSpaceArgv);

  deleteArgs(userSpaceArgs->argc,userSpaceArgs->argv);
  
  machine->Run();
}

/////////////////////////Halt/////////////////////////
void        syscallHalt(){
	DEBUG('m', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
}

/////////////////////////Exit/////////////////////////
void        syscallExit(int status){
  DEBUG('m', "Syscall Exit called with status: %d\n",status);
  if (status)
    printf("Thread %s exited with status: %d\n",currentThread->getName().c_str(),status);
  currentThread->Finish();
}

/////////////////////////Exec/////////////////////////
SpaceId     syscallExec(int argc, char** argv){
  DEBUG('m', "Syscall Exec called with excutable file: %s\n",argv[0]);

  OpenFile* executable = fileSystem->Open(argv[0]);
  if (executable == NULL){
    deleteArgs(argc,argv);
    return -1;
  }

  AddrSpace* space = new AddrSpace();

  Args* userSpaceArgs = new Args;
  int userSpaceArgv;
  
  if (!space->Initialize(executable, argc, argv, &userSpaceArgv)){
    delete executable;
    deleteArgs(argc,argv);
    delete space;
    return -1;
  }

  userSpaceArgs->argc = argc;
  userSpaceArgs->argv = argv;
  userSpaceArgs->userSpaceArgv = userSpaceArgv;
  
  Thread* child = new Thread(std::string(argv[0]),true);
  child->space = space;
  child->Fork( (VoidFunctionPtr) threadRun, (int)userSpaceArgs);
  
  // delete executable; //The addrspace destructor handles this now

  return (SpaceId) child;
}

/////////////////////////Join////////////////////////////
int         syscallJoin(SpaceId id){
  if (currentThread->isJoinableChild((Thread*) id))
    return ((Thread*)id)->Join();
  else
    return -1;
}

/////////////////////////Create/////////////////////////
void        syscallCreate(const char *name){
  currentThread->fdtable->create(std::string(name));
}

/////////////////////////Open/////////////////////////
OpenFileId syscallOpen(const char *name){
  return currentThread->fdtable->open(std::string(name));
}

/////////////////////////Write/////////////////////////
void        syscallWrite(char *buffer, int size, OpenFileId id){
  FileDescriptor descriptor = currentThread->fdtable->getDescriptor(id);
  if (descriptor.status == unused || descriptor.mode == r)
    return;

  if (descriptor.type == file){
    descriptor.file->Write(buffer,size);
  }

  if (descriptor.type == console){
    synchConsole->writeStr(std::string(buffer,size));
  }
}

/////////////////////////Read/////////////////////////
int         syscallRead(char *buffer, int size, OpenFileId id){
  FileDescriptor descriptor = currentThread->fdtable->getDescriptor(id);
  if (descriptor.status == unused || descriptor.mode == w){
    return 0;
  }

  if (descriptor.type == file){
    return descriptor.file->Read(buffer,size);
  }

  if (descriptor.type == console){
    synchConsole->readStr(buffer,size);
    return size;
  }

}

/////////////////////////Close/////////////////////////
void        syscallClose(OpenFileId id){
  currentThread->fdtable->close(id);
}

/////////////////////////Fork/////////////////////////
void        syscallFork(void (*func)()){
}

/////////////////////////Yield/////////////////////////
void        syscallYield(){
  currentThread->Yield();
}

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

void
ExceptionHandler(ExceptionType which)
{
  switch(which){

  case(SyscallException):      // A program executed a system call.
    SyscallHandler(machine->ReadRegister(2));
    break;
  case(NoException): // Everything ok!
    break;
  case(PageFaultException):    // No valid translation found
    PageFaultHandler(machine->ReadRegister(BadVAddrReg));    
    break;
  case(ReadOnlyException):     // Write attempted to page marked "read-only"
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(BusErrorException):     // Translation resulted in an invalid physical address
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(AddressErrorException): // Unaligned reference or one that
                					    // was beyond the end of the
		                			    // address space
    printf("Segmentation Fault: %d",which);
    ASSERT(FALSE);
    break;
  case(OverflowException):     // Integer overflow in add or sub.
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(IllegalInstrException): // Unimplemented or reserved instr.
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  default:
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
  }

}

void SyscallHandler(int type){

  int res;
  std::string arg;
  char* buf = NULL;
  char **argv;
  int argc;

  switch(type){
  case SC_Halt:
    syscallHalt();
    break;

  case SC_Exit:
    machine->WriteRegister(2,getArg(1));
    syscallExit(getArg(1));
    break;

  case SC_Exec:
    res = -1;
    if (readString(getArg(1), arg)){
      argc = tokenize(arg, argv);
      res = syscallExec(argc, argv);
      // deleteTokens(argc, argv); //We free this from the new thread
    }
    machine->WriteRegister(2,res);
    break;

  case SC_Join:
    res = syscallJoin((SpaceId)getArg(1));
    machine->WriteRegister(2,res);
    break;

  case SC_Create:
    if (readString(getArg(1), arg))
      syscallCreate(arg.c_str());
    break;

  case SC_Open:
    if (readString(getArg(1), arg))
      res = syscallOpen(arg.c_str());
    else 
      res = -1;
    machine->WriteRegister(2,res);
    break;

  case SC_Read:
    if (getArg(2) <= 0){
      machine->WriteRegister(2,-1);
      break;
    }
    buf = new char[getArg(2)];
    res = (int) syscallRead(buf , getArg(2),(OpenFileId) getArg(3));
    if (!writeBuffer(buf,res,getArg(1)))
      res = -1;
    //Set return value
    delete buf;
    machine->WriteRegister(2,res);
    break;

  case SC_Write:
    if (getArg(2) <= 0)
      break;
    buf = new char[getArg(2)];
    if (readBuffer(getArg(1), getArg(2), buf))
    {
   		syscallWrite(buf,getArg(2),(OpenFileId) getArg(3));
    }
    delete buf;
    break;

  case SC_Close:
    syscallClose((OpenFileId) getArg(1));
    break;

  case SC_Fork:
    break;

  case SC_Yield:
    syscallYield();
    break;

  default:
    printf("Unexpected syscall code %d\n", type);
    ASSERT(FALSE);
  }
      

    //Advance Program counter
    // int PrevPCRegVal = machine->ReadRegister(PrevPCReg);
    int PCRegVal = machine->ReadRegister(PCReg);
    int NextPCRegVal = machine->ReadRegister(NextPCReg);
    
    machine->WriteRegister(PrevPCReg, PCRegVal);
    machine->WriteRegister(PCReg, NextPCRegVal);
    machine->WriteRegister(NextPCReg, NextPCRegVal+4);
}

int getArg(int num){
  return  machine->ReadRegister(3+num);
}

bool readString(int virtAddr,std::string& str){
  str = std::string();

  int c = 1;
  bool res = true;

  while (res  && c != '\0'){
    res = ReadMem(virtAddr,1,&c);
    virtAddr++;
    str += (char)c;
  }

  return res;
}

bool readBuffer(int virtAddr, int size, char* buf){
  int rc;
  for (int i = 0; i < size ; i++,virtAddr++){
    if (!ReadMem(virtAddr, 1, &rc))
      return false;
    buf[i] = rc;
  }
  return true;
}


bool writeBuffer(char* buf, int size, int virtAddr){
  for (int i = 0; i < size ; i++,virtAddr++){
    if (!WriteMem(virtAddr, 1, buf[i]))
      return false;
  }
  return true;
}

bool
ReadMem(int addr, int size, int *value){
    int data;
    ExceptionType exception;
    int physicalAddress;
    
    DEBUG('a', "Reading VA 0x%x, size %d\n", addr, size);

    do {
      exception = machine->Translate(addr, &physicalAddress, size, FALSE);
      if (exception != NoException) {
	machine->WriteRegister(BadVAddrReg, addr);
	ExceptionHandler(exception);
      }
    } while (exception != NoException);

    switch (size) {
      case 1:
	data = machine->mainMemory[physicalAddress];
	*value = data;
	break;
	
      case 2:
	data = *(unsigned short *) &machine->mainMemory[physicalAddress];
	*value = ShortToHost(data);
	break;
	
      case 4:
	data = *(unsigned int *) &machine->mainMemory[physicalAddress];
	*value = WordToHost(data);
	break;

      default: ASSERT(FALSE);
    }
    
    DEBUG('a', "\tvalue read = %8.8x\n", *value);
    return (TRUE);
}

bool
WriteMem(int addr, int size, int value){
    ExceptionType exception;
    int physicalAddress;
     
    DEBUG('a', "Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

    do {
      exception = machine->Translate(addr, &physicalAddress, size, FALSE);
      if (exception != NoException) {
	machine->WriteRegister(BadVAddrReg, addr);
	ExceptionHandler(exception);
      }
    } while (exception != NoException);

    switch (size) {
      case 1:
	machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
	break;

      case 2:
	*(unsigned short *) &machine->mainMemory[physicalAddress]
		= ShortToMachine((unsigned short) (value & 0xffff));
	break;
      
      case 4:
	*(unsigned int *) &machine->mainMemory[physicalAddress]
		= WordToMachine((unsigned int) value);
	break;
	
      default: ASSERT(FALSE);
    }
    
    return TRUE;
  
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
      if (!item.empty() && item[0] != '\0')
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

// std::vector<std::string> tokenize(std::string str){
//   return split(str, ' ');
// }

int tokenize(std::string str, char **& tokens){
  std::vector<std::string> tks = split(str, ' ');
  tokens = new char*[tks.size()];
  for (int i = 0; i < tks.size(); i++){
    tokens[i] = new char[tks[i].size()+1];
    strcpy(tokens[i], tks[i].c_str());
  }
  return tks.size();
}


