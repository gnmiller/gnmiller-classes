#ifndef COREMAP_H
#define COREMAP_H

#include <bitset>
#include <map>
#include <set>
#include <list>
#include "syscall.h"

class CoreMap{

public:
    CoreMap();
    ~CoreMap();
private:

  std::set<Thread*> threadSet;


public:
  std::bitset<NumPhysPages> usedFrames;
  std::map<int,SpaceId> owner;//frame <--> Thread
  std::list<int> lru;

  void addThread(Thread* t);
  void delThread(Thread* t);
  bool onlyThread(){return threadSet.size() == 1;}

  void setUsed(int frame)  {lru.remove(frame); lru.push_back(frame);}
  void setUnused(int frame){lru.remove(frame);}
  int  getLRU(){return lru.front();}

};

#endif // COREMAP_H
