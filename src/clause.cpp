#include "clause.h"


Clause::Clause()
{
	numWatched = 0;
	index = -1;
    isSAT = false;
}

bool Clause::finalEval()
{
	//Loop through literals
	int litSize = _literals.size();
	for(int i = 0; i< litSize; i++)
	{
		if(_literals[i]->getResult() == 1)
		{
			return true;
		}
	}
	return false;
}

int Clause::watchedListIndex(Literal * lit)
{
	//First get the other watched
	if(watched[0] == lit)
	{
		return 0;
	}
	else if(watched[1] == lit)
	{
		return 1;
	}
	else
	{
		return ERROR;
	}
}

vector<Literal *> * Clause::getLiterals()
{
	return &_literals;
}
void Clause::addLiteral(Literal * lit)
{
	_literals.push_back(lit);
	recalcWeight();
}
void Clause::recalcWeight()
{
	//Let the weight of the clause be defined as 2^-|ωi|
	_weight = pow(2.0,-1.0*_literals.size());
}
float Clause::getWeight()
{
	return _weight;
}

int Clause::watchedListIndex(Variable * var)
{
	//Do for both pos and neg literals
	int posIndex = watchedListIndex(var->posLiteral);
	int negIndex = watchedListIndex(var->negLiteral);
	
	if(posIndex != ERROR)
	{
		return posIndex;
	}
	else if(negIndex != ERROR)
	{
		return negIndex;
	}
	else
	{
		return ERROR;
	}
}

BCPInfo Clause::getBCPInfo(Literal * currentWatched)
{
	//Need to populate:
	//	otherWatchedLit
	//	nonZeroResultNotOtherWatched
	
	//Return value
	BCPInfo rv;
	//Init values
	rv.otherWatchedLit = NULL;
	rv.nonZeroResultNotOtherWatched = NULL;
	
	//First get the other watched
	int watch_list_index = watchedListIndex(currentWatched);
	
	if(watch_list_index == ERROR)
	{
		debug(debugStream << "Error: currentWatched (index=" <<currentWatched->getIndex() << " is not one of the currently watched literals in clause" << index << endl ,1);
	}
	else
	{
		//Use the other index
		rv.otherWatchedLit = watched[watch_list_index == 1 ? 0 : 1];
	}
	
	//Loop through the clause literals
	int numLits = _literals.size();
	//Temp variable for ease of programming
	Literal * tempLit;
	for(int i = 0; i< numLits; i++)
	{
		tempLit = _literals[i];
		
		//nonZeroResultNotOtherWatched
		if( (tempLit->getResult() != 0) && 
            (tempLit != rv.otherWatchedLit) &&
            (tempLit != currentWatched))
		{
			rv.nonZeroResultNotOtherWatched = tempLit;
		}
	}
	
	return rv;
}

void Clause::print() 
{
	//Only print when debugging
	if(debug_level > 0)
	{
		cout << "clause " << index << ": ";
		cout << "(";
		int numLiterals = _literals.size();
		for (int i = 0; i < numLiterals; i++) {
			Literal* tempLiteral = _literals[i];

			if (tempLiteral->getPolarity() == NEG)
			{
				cout << "-";
			}

			cout << tempLiteral->getIndex() << "=" ;

			int result = tempLiteral->getResult();
			if (result == 1)
			{
				cout << "1";
			}
			else if (result == 0)
			{
				cout << "0";
			}
			else if (result == UNDEF)
			{
				cout << "U";
			}

			if (i != numLiterals - 1)
			{ 
				cout << ", ";
			}
		}
		cout << ")" << endl;
	}
}
