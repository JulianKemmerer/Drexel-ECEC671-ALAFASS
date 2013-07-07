#ifndef _VARIABLE_
#define _VARIABLE_

#include <vector>
#include "const.h"

using namespace std;

class Literal;
class Clause;

class Variable
{
  public:
    Variable();
    
    int getAssignment();
    void setAssignment(int ass);
    
    Literal * posLiteral;
    Literal * negLiteral;
    
    int index;
   
    //List where  this variable appears in positive form
    vector<Clause *> pos_clause_list; 
    //List where  this variable appears in negative form
    vector<Clause *> neg_clause_list;
    //List where this variable is watched in positive form
    vector<Clause *> pos_watched;
    //List where this variable is watched in positive form
    vector<Clause *> neg_watched;
    
	
	private:
		int _assignment;
	
};
#endif
