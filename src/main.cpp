//Compile with g++ -O2 main.cpp clause.cpp literal.cpp variable.cpp dpll.cpp -o ALAFASS

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include "const.h"

using namespace std;

//These are where the actual data for the program is stored
//The list of clauses that is the expression
//Expression typedef'd in clause.h
Expression the_expression;

int num_vars = 0;
int num_clauses = 0;
int num_backtracks = 0;
Variable* the_variables;

//Process the file and populate/link the clauses and variables
void processFile(ifstream & inputFile);
void processLine(string line, int clauseIndex);
void processVariable(int fileValue, Clause * clause, int clauseIndex);
void printAssignments();
void doubleCheckAssignments();

int DPLL(Variable &justAssigned);
int
HeuristicallyChooseAnUnassignedVariableXAndHeuristicallyChooseAValueV
(Variable **x, int &v);

void debug(ostream& dataOut, int level);
ostringstream debugStream;
//ostringstream::out
int debug_level = 0;

//Stack overflow split code
//http://stackoverflow.com/questions/236129/splitting-a-string-in-c
vector<string> &split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);


int main(int argc, char *argv[]) 
{
	//Check that two arguments exist
	if(argc < 2) 
	{
		debug(debugStream << "Please supply a file to read." << endl ,0);
		return -1;
	} 
	else 
	{
		//Find out which arg to use
		//Loop through each arg
		int optArg = -1;
		int fileArg = -1;
		for(int i = 1; i< argc; i++)
		{
			string arg = string(argv[i]);
			if(arg.find("-l") != -1)
			{
				//This arg is option
				optArg = i;
			}
			else
			{
				//This arg is file
				fileArg = i;
			}
		}
		
		//Get the debug level if one
		//Assume single digit
		if(optArg!=-1)
		{
			//Option found, defult with option = 1
			debug_level = 1;
			string optLString = string(argv[optArg]);
			int lIndex = optLString.find("-l");
			if(optLString.length() > 2)
			{
				//Get digit
				lIndex +=2;
				optLString = optLString.substr(lIndex,1);
				debug_level = atoi(optLString.c_str());
			}
		}
		debug(debugStream << "Debug level: " << debug_level << endl ,0);
		
		
		if(fileArg != -1)
		{
			debug(debugStream << "Input Filename: " << argv[fileArg] << endl ,0);
			//Try to open the file
			ifstream inputFile( argv[fileArg] );
			//Suceeded in opening?
			if ( !inputFile.is_open() ) 
			{
			  debug(debugStream << "Could not open file: " << argv[fileArg] << endl ,0);
			  return -1;
			} 
			else 
			{	
			  //Begin reading the file, get the expression
			  processFile(inputFile);
			}
			//the file is closed implicitly here
		}
		else
		{
			debug(debugStream << "Please supply a file to read." << endl ,0);
			return -1;
		}
	}
	
	debug(debugStream << "Completed reading the input file." << endl ,1);
	//SAT SOLVE - ALAFASS AWAY!
	debug(debugStream << "ALAFASS AWAY!" << endl ,1);

    Variable* X = NULL;
    int V = 0;
    HeuristicallyChooseAnUnassignedVariableXAndHeuristicallyChooseAValueV
        (&X, V);

    (*X).setAssignment(V);
	debug(debugStream << "Main initial heuristic assignment: x" << X->index << "=" << V << endl ,1);
    int result = DPLL(*X);
	if (result == SAT)
	{
		debug(debugStream << "Main initial heuristic assignment: x" << X->index << "=" << V << "is SAT" << endl ,1);
		debug(debugStream << "p sat " << num_vars << " " << num_clauses << endl ,0);
		printAssignments();
	}
	else if (result == UNSAT)
	{
        (*X).setAssignment(!V);
        debug(debugStream << "Main initial heuristic assignment: x" << X->index << "=" << V << "is UNSAT" << endl ,1);
        debug(debugStream << "Main secondary heuristic assignment: x" << X->index << "=" << !V << endl ,1);
        result = DPLL(*X);
        if (result == SAT) 
        {
			debug(debugStream << "Main secondary heuristic assignment: x" << X->index << "=" << !V << "is SAT" << endl ,1);
            debug(debugStream << "p sat " << num_vars << " " << num_clauses << endl ,0);
            printAssignments();
        }
        else
        {
            //The expression could not be satisfied by either assignment 
            //of the initial variable
            debug(debugStream << "Main secondary heuristic assignment: x" << X->index << "=" << !V << "is UNSAT" << endl ,1);
            debug(debugStream << "p unsat " << num_vars << " " << num_clauses << endl ,0);
        }
	}
	
    doubleCheckAssignments();
    debug(debugStream << "Backtracks: " << num_backtracks << endl ,0);
	
  return 0;
}

void debug(ostream& dataOut, int level)
{
	//Cast to a string stream
	ostringstream * oss = dynamic_cast<ostringstream *>(&dataOut);
	
	//If debug level is high enough for this
	if(debug_level>= level)
	{
		//Print the stuff
		cout << oss->str();
		oss->str("");
	}
	else
	{
		//Don't print just clear
		oss->str("");
	}
}

void doubleCheckAssignments()
{
	//Loop through clauses
	for(int i=0; i < num_clauses; i++)
	{
		if(the_expression[i].finalEval() == false)
		{
			debug(debugStream << "UNSAT Verified." << endl ,0);
			return;
		}
	}
	debug(debugStream << "SAT Verified." << endl ,0);
}

void printAssignments()
{
	//Print in the way the project description defines
	for (int i = 1; i <= num_vars; ++i) 
	{
        debug(debugStream << i << " " << the_variables[i].getAssignment() << endl ,0);
    }
}

void processFile(ifstream & inputFile)
{
	int numVars = 0;
	int numClauses = 0;
	
	//Loop through each line in file
	//Count which clause we are on
	int clauseIndex = 0;
	for(string line; getline(inputFile, line); ) 
	{
		if(line[0] =='c')
		{
			//If comment line
			//Ignore
		}
		else if(line[0] == 'p')
		{
			//Get preamble info
			//p cnf 200 1200
			//Split the line
			vector<string> toks;
			split(line,' ',toks);
			//Index 2 is #vars, index 3 is #clauses
			numVars = atoi(toks[2].c_str());
            num_vars = numVars;
			numClauses = atoi(toks[3].c_str());
            num_clauses = numClauses;
			
			//Allocate the_expression size
			the_expression = new Clause[numClauses];
			
			//Allocate the variables array size
			//+1 to maintain easy indexing
			the_variables = new Variable[numVars +1];
			
			debug(debugStream << num_vars << " variables allocated." << endl ,1);
			debug(debugStream << num_clauses << " clauses allocated." << endl ,1);
		}
		//If the line starts with a number or the minus sign
		else if( (line[0] >= '0' && line[0] <= '9') || line[0]=='-')			  
		{
			//Continue processing data
			processLine(line, clauseIndex);
			clauseIndex++;
		}
		else
		{
			//Unknown line starting char
			debug(debugStream << "Input line: " << line << endl << 
			"begins with unknown character: " << line[0] << endl ,1);
		}
	}
}

void processLine(string line, int clauseIndex)
{
	//Working with a specific clause now
	//Adding data to clause at the_expression[clauseIndex]
	//Loop through each line and make a literal for each integer
	vector<string> vals;
	split(line,' ',vals);
	int valLen = vals.size();
	
	for(int i = 0; i< valLen; i++)
	{
		//Process the int into a variable and add it to the clause
		int val = atoi( vals[i].c_str() );
		if (val!=0) //0 is end of line
		{
			//Give positive/negative for further processing
			//Also give the point to the clause object in question
			processVariable(val,&(the_expression[clauseIndex]), clauseIndex);
		}
	}
}

void processVariable(int fileValue, Clause * clause, int clauseIndex)
{
	//Which variable is being pointed to:
	//ABS of file read value is index into variables array
	Variable * varPtr = &(the_variables[abs(fileValue)]);
	varPtr->index = abs(fileValue);
	//debug(debugStream << "						Setting variable index=" << varPtr->index << endl ,1);
	clause->index = clauseIndex;
	
	//Add to lists linking variables and clauses
	if(fileValue >0) //Positive literal
	{	
		//Add to positive lists
		varPtr->pos_clause_list.push_back(clause);
		if(varPtr->posLiteral ==NULL)
		{
			varPtr->posLiteral = new Literal();
			varPtr->posLiteral->parentVariable = varPtr;
            varPtr->posLiteral->setPolarity(POS);
		}
		clause->addLiteral(varPtr->posLiteral);
		
		//Manage initial watched variables
		if(clause->numWatched < 2)
		{
			clause->watched[clause->numWatched] = varPtr->posLiteral;
			//debug(debugStream << "Clause " << clauseIndex << " watched" << 
				//clause->numWatched << " = " << varPtr->posLiteral->getIndex() << endl ,1);
			varPtr->pos_watched.push_back(clause);
            //debug(debugStream << "pushing the following clause onto variable " << varPtr->index 
            //     << "'s positive watch list: " << endl ,1);
            //clause->print();
			clause->numWatched++;
		}
	}
	else //Negative literal
	{
		//Add to negative list
		varPtr->neg_clause_list.push_back(clause);
		if(varPtr->negLiteral == NULL)
		{
			varPtr->negLiteral = new Literal();
			varPtr->negLiteral->parentVariable = varPtr;
            varPtr->negLiteral->setPolarity(NEG);
		}
		clause->addLiteral(varPtr->negLiteral);
		
		//Manage initial watched variables
		if(clause->numWatched < 2)
		{
			clause->watched[clause->numWatched] = varPtr->negLiteral;
			debug(debugStream << "Clause " << clauseIndex << " watched" << 
				clause->numWatched << " = " << varPtr->negLiteral->getIndex() << endl ,1);
			varPtr->neg_watched.push_back(clause);
            //debug(debugStream << "pushing the following clause onto variable " << varPtr->index 
            //     << "'s negative watch list: " << endl ,1);
            //clause->print();
			clause->numWatched++;
		}
	}
	
	
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}
