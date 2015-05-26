/**
 * Greg Miller -- gnmiller
 * CS4411 Spring 15
 * Project 2
 * 
 * Due: 2/3/15 09:35
 * Submitted: 2/4/15 00:05
 * 
 * Slip Days used: 1
 * Slip Days Remaining: 4
 * 
 * File: helper.h
 * 
 * Contains prototypes for Nodes used in the lists
 * that refstats uses to track various data while processing
 * the logs. Also contains the prototype for check_ip().
 */

class Node {
  public:
    Node();
    Node( char* );
    char *data;
};

class IPNode: public Node {
  public:
     IPNode( char*, char* );
	 IPNode( IPNode *i );
     char *data2;
};

class RNode: public IPNode {
  public:
     RNode( char*, char*, int );
     int o;
};

int check_ip( char* ip );