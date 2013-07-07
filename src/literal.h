#ifndef _LITERAL_
#define _LITERAL_

#include "const.h"

using namespace std;

class Variable;

class Literal
{
  public:
    Literal();
    //Some of these are for ease of programming - just reference the
    //parent variable
    int getAssignment();
    void setAssignment(int ass);
    int getResult();
  
	int getPolarity();
	void setPolarity(int pol);
    
    Variable * parentVariable;
    
    int getIndex();
    void setIndex(int i);
    
    void recalcResult();
    
	
private:
	int _result;
	int _assignment;
    int _polarity;
    int _index;
};
#endif
