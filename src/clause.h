#ifndef _CLAUSE_
#define _CLAUSE_
#include "const.h"
#include <iostream>
#include <cmath>
#include <sstream>

class Variable;

using namespace std;
extern void debug(ostream& dataOut, int level);
extern int debug_level;

extern ostringstream debugStream;

struct BCPInfo
{
	Literal * otherWatchedLit;
	Literal * nonZeroResultNotOtherWatched;
	Literal * unassignedNotWatched;
	Literal * nonZeroResult; //try not to be otherWatchedLit
	//Want to be unwatched completely
	//Cannot be the currently watched lit
};
typedef struct BCPInfo BCPInfo;

class Clause
{
	public:
		Clause();
		Literal * watched[2];
		vector<Literal *> * getLiterals();
		void addLiteral(Literal * lit);
		//Jeroslaw Wang Weight
		float getWeight();
		void recalcWeight();
		int numWatched;
		int index;
        bool isSAT;
		
		//Returns BCPInfo struct full of info
		BCPInfo getBCPInfo(Literal * currentWatched);
		
		//Get the index in the watched list
		int watchedListIndex(Literal * lit);
		int watchedListIndex(Variable * var);
	
        //print the clause and its current assignments	
        void print();
        //Final double check
        bool finalEval();
		
	private:
		float _weight;
		vector<Literal *> _literals;
		
		
};

typedef Clause* Expression;
#endif
