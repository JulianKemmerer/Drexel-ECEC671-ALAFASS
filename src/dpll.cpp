#ifndef _DPLL_
#define _DPLL_
#include "const.h"
#include <iostream>
#include <queue>

using namespace std;

extern int num_vars;
extern int num_clauses;
extern int num_backtracks;
extern Variable* the_variables;
extern Expression the_expression;
extern void debug(ostream& dataOut, int level);
extern ostringstream debugStream;
extern int debug_level;

struct changes
{
    vector<Variable*> variables;
    vector<Clause*> clauses;
};

bool isClauseSAT (Clause *clause);

int BCP (Variable &justAssigned, struct changes &changeList);
int 
HeuristicallyChooseAnUnassignedVariableXAndHeuristicallyChooseAValueV
    (Variable **x, int &v);
void undoChanges (struct changes &changeList);
vector<Clause *> * getAppropriateWatchList(Variable * varToCheck);
void case1(Clause * clause,
           int varToCheckWatchedListIndex,
           Literal * nonZeroResultNotOtherWatched);
int case234(Clause * clause, Literal * nonZeroResult,
	Literal * otherWatchedLit );
void case2(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications );
void case3(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications );
void case4(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications );
	
int DPLL(Variable &justAssigned)
{
    //Record all variables whose values are changed during this BCP run
    //A change in the change list is a pointer a variable 
	//Undone by setting variable to UNDEF
    struct changes changeList;
    
    //Do BCP and get result
    int result = BCP(justAssigned, changeList);
    if (result == UNSAT) {
        return UNSAT;
    } else if (result == ERROR) {
        debug(debugStream << "Error: Variable passed to BCP was not assigned!" << endl ,1);
        return ERROR;
    } else if (result == DECIDE) {
        // Do nothing, the heuristic is about to run
		debug(debugStream << "Decide! NOW!" << endl ,1);
    }

    Variable* X = NULL;
    int V = 0;
    result = 
       HeuristicallyChooseAnUnassignedVariableXAndHeuristicallyChooseAValueV(
           &X, V);
    if (result == SAT) {
        debug(debugStream << "Heuristic choice returned SAT!" << endl ,1);
        return SAT;
    }

    (*X).setAssignment(V);

    if (DPLL(*X) == SAT) {
		debug(debugStream << "Initial  of variable" << X->index << " to " << V << " is SAT" << endl ,1);
        return SAT;
    } else {
		debug(debugStream << "Initial  of variable" << X->index << " to " << V << " is UNSAT" << endl ,1);
        (*X).setAssignment(V ? 0 : 1);
		debug(debugStream << "    assignment changed to " << (*X).getAssignment() << endl ,1);
        result = DPLL(*X);
        if (result == SAT) {
			debug(debugStream << "Second assignment of variable" << X->index << " to " << V << " is SAT" << endl ,1);
            return SAT;
        } else if (result == UNSAT) {
            changeList.variables.push_back(X);
            //undo change list
            undoChanges(changeList);
			debug(debugStream << "Second assignment of variable" << X->index << " to " << V << " is UNSAT" << endl ,1);
            return UNSAT;
        } else if (result == ERROR) {
            return ERROR;
        }
    }
}

int BCP (Variable &justAssigned, struct changes &changeList)
{
	//Do BCP on expression, which includes 2-way variable watch lists
    /*
     * Returns one of:
     *     ERROR, if justAssigned was not actually assigned
     *     UNSAT, if the initial assignment is unsatisfiable
     *     DECIDE, if BCP is "asking" for another descision
     */

    //This queue stores variables that are assigned during this BCP run
    queue<Variable*> implications;
    
    //Prime queue with initial implication
    implications.push(&justAssigned);
    
    //While still implications to check
	while(!(implications.empty()) )
	{
		//Which variable are we dealing with?
		Variable * varToCheck = implications.front();
	    implications.pop();
	
		//Find out which watch list we are using
		//Get the appropriate watch list
        debug(debugStream << "Running BCP on variable: " << varToCheck->index << endl ,1);
		vector<Clause *> * watch_list = 
			getAppropriateWatchList(varToCheck);
		if(watch_list == NULL)
		{
			debug(debugStream << "Variable" << varToCheck->index << " was not just assigned - we can't choose a watch list" << endl ,1);
			return ERROR;
		}
		
		//Temp clause variable
		Clause * tempClause;
        vector<Clause*>::iterator it = (*watch_list).begin();
        debug(debugStream << "Looping through watch list for x" << varToCheck->index << endl ,1);
		while(it != (*watch_list).end())
		{
			tempClause = (*it);
			debug(debugStream << "    Looking at clause" <<  tempClause->index << endl ,1);
            tempClause->print();

            if (isClauseSAT(tempClause))
            {
                //Mark this clause as satisfied
                tempClause->isSAT = true;

                //Add the clause to the changeList, se we can backtrack if needed
                changeList.clauses.push_back(tempClause);
            }

            if (tempClause->isSAT) {
                debug(debugStream << "    Clause" <<  tempClause->index << "is SAT already" << endl ,1);
                ++it;
                continue;
            }
			
			//varToCheck is one of the watched literals in this clause
			//Which one?
			int varToCheckWatchListIndex = 
				tempClause->watchedListIndex(varToCheck);
			if(varToCheckWatchListIndex == ERROR)
			{
				debug(debugStream << "		Error: varToCheck (index=" <<varToCheck->index << " is not one of the currently watched literals in clause" << tempClause->index << endl ,1);
				return ERROR;
			}

            //This clause has only this one watched literal
            if (tempClause->watched[varToCheckWatchListIndex == 1 ?  0 : 1] == NULL)
            {
                debug(debugStream << "		Literal " << varToCheck->index
                     << " has a value of 0 in a clause with only one literal" << endl ,1);
                undoChanges(changeList);
                return UNSAT;
            }
			
			//Get the bcp info for this clause		
			//Must supply the currently watched
			BCPInfo info = tempClause->
				getBCPInfo(tempClause->watched[varToCheckWatchListIndex]);
				
			//Use this BCP info to decide cases
			/* Here is what is provided
			struct BCPInfo
			{
				Literal * otherWatchedLit;
				Literal * nonZeroResultNotOtherWatched;
			};
			*/
			//Where a NULL pointer indicates 
			//that such a literal does not exist
			
			//Case (1): If there exists another unassigned literal 
			//y that is not 0-valued and 
			//it is not the other watched literal
			//How can it be unassigned and not (not zero valued)?
			//Same as:
			//How could a literal be unassigned and zero valued?
			//Take this as nonZeroResultNotOtherWatched
			if(info.nonZeroResultNotOtherWatched != NULL)
			{
				debug(debugStream << "        Clause" <<  tempClause->index << " is case1." << endl ,1);
				//Case 1 is true
				//Do case 1
				case1(tempClause,
                      varToCheckWatchListIndex,
                      info.nonZeroResultNotOtherWatched);
                it = (*watch_list).erase(it);
			}
			
			//Case(2,3,4): There does not exist another such literal y 
			//which is unassigned
			//Case 2,3,4 use the same condition but then check 
			//3 different supplement conditions
			else
			{
				//Check the three cases
				//Returns 2,3,4 for case # or ERROR
				int case234result = case234(tempClause, 
					info.nonZeroResult, info.otherWatchedLit);
				
				//Process here!!!
				if(case234result ==2)
				{
					debug(debugStream << "        Clause" <<  tempClause->index << " is case2." << endl ,1);
					case2(tempClause, info, changeList, implications);
				}
				else if(case234result ==3)
				{
					debug(debugStream << "        Clause" <<  tempClause->index << " is case3." << endl ,1);
					case3(tempClause,info,changeList,implications);
				}
				else if(case234result ==4)
				{
					debug(debugStream << "        Clause" <<  tempClause->index << " is case4." << endl ,1);
					case4(tempClause,info,changeList,implications);
                    return UNSAT;
				}
				else
				{
					debug(debugStream << "        Error in deciding between case 2,3,4!" << endl ,1);
				}
                ++it;
			}
		}
	}

    return DECIDE;
}

void case1(Clause * clause, 
           int varToCheckWatchedListIndex, 
           Literal * nonZeroResultNotOtherWatched)
{
	debug(debugStream << "        Case 1 Clause" <<  clause->index << ": Switch one of the clause's watched literals to the newly identified literal" << endl ,1);
    //Switch one of the clause's watched literals to the newly identified literal
	clause->watched[varToCheckWatchedListIndex] = nonZeroResultNotOtherWatched;
   
    //Add this clause to that vairable's watch list 
    Variable* var = nonZeroResultNotOtherWatched->parentVariable;
    int newWatchedPolarity = nonZeroResultNotOtherWatched->getPolarity();

    if (newWatchedPolarity == POS)
    {
        debug(debugStream << "        Pushing clause" << clause->index << " onto x" 
             << var->index << "'s positive watch list" << endl ,1);
        var->pos_watched.push_back(clause);
    }
    else
    {
        debug(debugStream << "        Pushing clause" << clause->index << " onto x" 
             << var->index << "'s negative watch list" << endl ,1);
        var->neg_watched.push_back(clause);
    }
}

void case2(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications )
{

	debug(debugStream << "        Case 2 Clause" <<  clause->index << ": Unit clause, we assign a variable as a new implication" << endl ,1);

    //Unit clause, we assign a variable as a new implication
    if (info.otherWatchedLit->getPolarity() == POS) 
    {
		debug(debugStream << "        x"<<info.otherWatchedLit->getIndex() << " is positive polarity" << endl ,1);
        debug(debugStream << "        setting the assignment for x"
             << info.otherWatchedLit->parentVariable->index << "=1" << endl ,1);
        info.otherWatchedLit->setAssignment(1);
        clause->print();
    }
    else
    {
		debug(debugStream << "        x"<<info.otherWatchedLit->getIndex() << " is negitive polarity" << endl ,1);
        debug(debugStream << "        setting the assignment for x"
             << info.otherWatchedLit->parentVariable->index << "=0" << endl ,1);
        info.otherWatchedLit->setAssignment(0);
        clause->print();
    }

    //Add the variable to the changeList, so we can backtrack if needed
    changeList.variables.push_back(info.otherWatchedLit->parentVariable);

    //Add the variable to the implications queue
    implications.push(info.otherWatchedLit->parentVariable);

    //Mark this clause as satisfied
    clause->isSAT = true;

    //Add the clause to the changeList, se we can backtrack if needed
    changeList.clauses.push_back(clause);
}

void case3(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications )
{
	debug(debugStream << "		Case 3 Clause" <<  clause->index << ": Mark this clause as satisfied" << endl ,1);

    //Mark this clause as satisfied
    clause->isSAT = true;

    //Add the clause to the changeList, se we can backtrack if needed
    changeList.clauses.push_back(clause);
}

void case4(Clause * clause, BCPInfo	info, struct changes &changeList,
	queue<Variable*> &implications )
{
	debug(debugStream << "		Case 4 Clause" <<  clause->index << ": UNSAT, undo changes" << endl ,1);

    undoChanges(changeList);
}

int case234(Clause * clause, Literal * nonZeroResult,Literal * otherWatchedLit )
{
	if(otherWatchedLit == NULL)
	{
		//Must be another watched lit at this stage
		return ERROR;
	}
	
	//This is where nonZeroResult comes in
	//- trying not to be otherWatchedLit if possible
	//- Want to be unwatched completely if possible
	//- Cannot be the currently watched lit
	
	//Check for which case this is:
	
	//Case(2) supplement condition:
	//What if the only other not-0-valued literal is our other 
	//watched literal, and it’s a free literal (unassigned)?
	if(otherWatchedLit->getResult() == UNDEF)
	{
		//Case 2 is true
		return 2;
	}
	
	//Case(3) supplement condition:
	//Now, the only other not-0-valued literal 
	//is our other watched literal, but it’s assigned to 1?
	if(otherWatchedLit->getResult() == 1)
	{
		//Case 3 is true
		return 3;
	}
	
	//Case(4) supplement condition:
	//Now, there is no other not-0-valued literal at all.
	if(otherWatchedLit->getResult() == 0)
	{
		//Case 4 is true
		return 4;
	}
	
	return ERROR;
}


vector<Clause *> * getAppropriateWatchList(Variable * varToCheck)
{
	if(varToCheck->getAssignment() == 0)
	{
		debug(debugStream << "    was assigned to 0" << endl ,1);
		return &(varToCheck->pos_watched);
	}
	else if(varToCheck->getAssignment() == 1)
	{
		debug(debugStream << "    was assigned to 1" << endl ,1);
		return &(varToCheck->neg_watched);
	}
	else
	{
		return NULL;
	}
}

void undoChanges (struct changes &changeList) {
    //Back track!!!
    num_backtracks++;
	debug(debugStream << "Undoing changes." << endl ,1);

    //Loop through the list of changed variables
    //Set each assignment to UNDEF
    int varChangeSize = changeList.variables.size();
    for(int i = 0; i < varChangeSize; i++)
    {
		debug(debugStream << "	variable " << changeList.variables[i]->index << " set to undefined." << endl ,1);
        changeList.variables[i]->setAssignment(UNDEF);
    }

    //Loop through the list of changed clauses
    //Set each clause to isSat = false
    int clauseChangeSize = changeList.clauses.size();
    for(int i = 0; i < clauseChangeSize; i++)
    {
		debug(debugStream << "	clause " << changeList.clauses[i]->index << " set to not SAT." << endl ,1);
        changeList.clauses[i]->isSAT = false;
    }
}

int
HeuristicallyChooseAnUnassignedVariableXAndHeuristicallyChooseAValueV
(Variable **x, int &v)
{
	//Do Jeroslaw-Wang Scoring
	//Return choice via x and v passed by reference
    //Return SAT if the function is satisfied
    
    /*
	  Suppose x is a literal that appears in our clauses {ωi | i=1,2,...n}
	  To score x, add up weight of each clause that literal x appears in
	  score(x) = Σ (clauses ωi with x) [ weight ]
	  Do this for both polarities: compute score(x) and separately score( ¬ x )
	  Pick unassigned literal with the largest J-W score
	*/

	float posScore[num_vars+1];
	float negScore[num_vars+1];
	
	//Loop through all variables
	//Temp var for easy programming
	Variable * tempVar;
	//Lists to be used
	vector<Clause * > * pos_clause_list;
	vector<Clause * > * neg_clause_list;
	int list_size = 0;
	int clauseIndex = 0;
	//Allocate space for score
	float maxPosScore = -999999999;
	Variable * maxPosScoreVar = NULL;
	float maxNegScore = -999999999;
	Variable * maxNegScoreVar = NULL;
	float tempSum = 0;
    for(int varIndex = 1; varIndex	<=num_vars; varIndex++)
    {
		tempVar = & (the_variables[varIndex]);
		
		//Only do this for unassigned vars, since only their score
		//Are the ones we care about
		if(tempVar->getAssignment() == UNDEF)
		{
			//Get lists of where this variable appears
			pos_clause_list = &(tempVar->pos_clause_list);
			neg_clause_list = &(tempVar->neg_clause_list);
			
			//Loop through pos clause list
			list_size = pos_clause_list->size();
			tempSum = 0;
			for(clauseIndex = 0; clauseIndex < list_size; clauseIndex++)
			{
				//Sum score for this variable
				tempSum += 
					(*pos_clause_list)[clauseIndex]->getWeight();
			}
			//Finished looping for this positive var
			//Is it larger than the max?
			if(tempSum > maxPosScore)
			{
				maxPosScore = tempSum;
				maxPosScoreVar = tempVar;
			}
			
			//Now loop through neg clause list
			list_size = neg_clause_list->size();
			tempSum = 0;
			for(clauseIndex = 0; clauseIndex < list_size; clauseIndex++)
			{
				//Sum score for this variable
				tempSum += 
					(*neg_clause_list)[clauseIndex]->getWeight();
			}
			//Finished looping for this negative var
			//Is it larger than the max?
			if(tempSum > maxNegScore)
			{
				maxNegScore = tempSum;
				maxNegScoreVar = tempVar;
			}
		}

        /*
        debug(debugStream << "After running the Heuristic for variable: " << tempVar->index << endl ,1);
        debug(debugStream << "   The maximum negative score is: " << maxNegScore << endl ,1);
        debug(debugStream << "   The maximum positive score is: " << maxPosScore << endl ,1);
        debug(debugStream << "   The maximum negative score var is: " << maxNegScoreVar << endl ,1);
        debug(debugStream << "   The maximum positive score var is: " << maxPosScoreVar << endl ,1);
        */
	}
	
	//Variable chosen?
	if( (maxPosScoreVar==NULL) && (maxNegScoreVar==NULL) )
	{
		//Uh oh no chosen variable?
		//SAT?
		//That means there were no unassigned variables to check
		//SAT?
		debug(debugStream << "Heurestic: No variable chosen. SAT!" << endl ,1);
		return SAT;
	}
	else
	{
		//Pick the max one
		if(maxPosScore > maxNegScore)
		{
			(*x) = maxPosScoreVar;
			v = 1;
		}
		else
		{
			(*x) = maxNegScoreVar;
			v = 0;
		}
		
		//Now return something to indicate just a choice was made
		//?UNSAT?
		debug(debugStream << "Heurestic variable chosen: x" << (*x)->index << "=" << v<< endl ,1);
		return UNSAT;
	}
}

bool isClauseSAT (Clause *clause)
{
    bool clause_sat = false;

    vector<Literal*>* literals = clause->getLiterals();

    int numLiterals = (*literals).size();
    for (int i = 0; i < numLiterals; i++)
    {
        Literal* literal = (*literals)[i];
        if (literal->getResult() == 1)
        {
            clause_sat = true;
            break;
        }
    }

    return clause_sat;
}
#endif
