#include "literal.h"

Literal::Literal()
{
	 _result = UNDEF;
	 _assignment = UNDEF;
	 _polarity = UNDEF;
	 _index = -1;
}

int Literal::getAssignment()
{
	parentVariable->getAssignment();
}
void Literal::setAssignment(int ass)
{
	parentVariable->setAssignment(ass);
}
int Literal::getResult()
{
	return _result;
}

int Literal::getPolarity()
{
	return _polarity;
}
void Literal::setPolarity(int pol)
{
	_polarity = pol;
	recalcResult();
}

int Literal::getIndex()
{
	return parentVariable->index;
}
void Literal::setIndex(int i)
{
	parentVariable->index = i;
}

void Literal::recalcResult()
{
	_assignment = parentVariable->getAssignment();
	
	if(_assignment == 1)
	{
		if(_polarity == POS)
		{
			_result = 1;
		}
		else //NEG polarity
		{
			_result = 0;
		}
	}
	else if( _assignment == 0)
	{
		if(_polarity == POS)
		{
			_result = 0;
		}
		else //NEG polarity
		{
			_result = 1;
		}
	}
	else
	{
		_result = UNDEF;
	}
}
