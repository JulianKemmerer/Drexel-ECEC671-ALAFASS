#include "variable.h"
#include <iostream>

Variable::Variable()
{
	_assignment = UNDEF;
	posLiteral = NULL;
    negLiteral = NULL;
	index = 0;
}

int Variable::getAssignment()
{
	return _assignment;
}
void Variable::setAssignment(int ass)
{
	_assignment = ass;
	if(posLiteral != NULL)
	{
		posLiteral->recalcResult();
	}
	if(negLiteral != NULL)
	{
		negLiteral->recalcResult();
	}
}
