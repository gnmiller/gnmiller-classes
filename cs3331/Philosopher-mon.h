//------------------------------------------------------------------------
// Filename: 
//     Philosopher-mon.h
// PROGRAM DISCRIPTION
//     Interface file for PhilosopherMonitor thread class
//
//    MONITOR  philosopher, Hoare Style:                                               
//    This monitor is for a version of the philosopher problem.  
//    It consists of the following procedures:                                     
//    (1) CanEat(int Number)    - if the chopstick of philosopher
//                                           Number is available
//    (2) GetChopStick(int i)   - philosopher i want chopsticks
//    (3) PutChopStick(int i)   - philosopher i put chopsticks down
//------------------------------------------------------------------------

#ifndef  _PHILOSOPHER_MON_H
#define  _PHILOSOPHER_MON_H

#include "ThreadClass.h"

#define AVAILABLE        1
#define NOT_AVAILABLE    0

//------------------------------------------------------------------------
// PhilosopherMonitor class definition
//------------------------------------------------------------------------

class PhilosopherMonitor: public Monitor 
{
     public:
          PhilosopherMonitor(char* Name);

          void  GetChopsticks(int Number);
          void  PutChopsticks(int Number);

     private:
          Condition *WaitChopStick[PHILOSOPHERNUM];
          int ChopStick[PHILOSOPHERNUM];
          int CanEat(int Number);
};

#endif