//------------------------------------------------------------------------
// Filename: 
//     Philosopher-mon.cpp
// PROGRAM DISCRIPTION
//     PhilosopherMonitor class implementation file
//------------------------------------------------------------------------

#include <iostream.h>
#include <stdlib.h>
#include "ThreadClass.h"
#include "Philosopher-Thrd.h"
#include "Philosopher-mon.h"

//------------------------------------------------------------------------
// PhilosopherMonitor::PhilosopherMonitor()
//      constructor 
//------------------------------------------------------------------------

PhilosopherMonitor::PhilosopherMonitor(char* Name)
                    : Monitor(Name, HOARE) 
{
     for (int i = 0; i < PHILOSOPHERNUM; i++) {
          strstream *ConditionName;
          ConditionName = new strstream;
          ConditionName->seekp(0, ios::beg);
          (*ConditionName) << "WaitChopStick" << i << '\0';
          WaitChopStick[i] = new Condition(ConditionName->str());
          ChopStick[i] = AVAILABLE;
     }
}

//------------------------------------------------------------------------
// FUNCTION  CanEat(int):
//   This function checks to see if both chopsticks are availavle
//   to philosopher 'Number'
//   int Number     --   The number of the philosopher
//------------------------------------------------------------------------

int PhilosopherMonitor::CanEat(int Number)
{
     if ((ChopStick[Number] == AVAILABLE) && 
         (ChopStick[(Number+1)%PHILOSOPHERNUM] == AVAILABLE))
          return 1;      // if both chopsticks are available, return 1
     else
          return 0;      // otherwise, return 0
}

//------------------------------------------------------------------------
// FUNCTION  GotChopsticks(int):
//   int Number     --   The number of the philosophers
//------------------------------------------------------------------------

void PhilosopherMonitor::GetChopsticks(int Number)
{
     MonitorBegin();
     while (!CanEat(Number))                 // wait if both chops 
          WaitChopStick[Number]->Wait();     // are not available
     ChopStick[Number] = NOT_AVAILABLE;      // got them, set to unavailable
     ChopStick[(Number+1)%PHILOSOPHERNUM] = NOT_AVAILABLE;
     MonitorEnd();
}

//------------------------------------------------------------------------
// FUNCTION  PutChopsticks(int):
//    This function implement the philosopher put back two chopsticks
//   int Number     --   The number of the philosopher
//------------------------------------------------------------------------

void  PhilosopherMonitor::PutChopsticks(int Number)
{
     MonitorBegin();
     ChopStick[Number]=AVAILABLE;       // put back two chopsticks
     ChopStick[(Number+1)%PHILOSOPHERNUM]=AVAILABLE;
     WaitChopStick[(Number+1)%PHILOSOPHERNUM]->Signal();  // right neighbor
     WaitChopStick[(Number+PHILOSOPHERNUM-1)%PHILOSOPHERNUM]->Signal(); // left
     MonitorEnd();
}

// end of Philospher-mon.cpp