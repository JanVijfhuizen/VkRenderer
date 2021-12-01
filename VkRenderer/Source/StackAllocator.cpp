#include "pch.h"
#include "StackAllocator.h"

StackAllocator::StackAllocator()
{
	_current = new Block;
}

StackAllocator::~StackAllocator()
{
	while(_current)
	{
		Block* previous = _current->previous;
		delete _current;
		_current = previous;
	}
}

void StackAllocator::Pop()
{
	if(_current->offset == 0)
	{
		assert(_current->previous);
		Block* previous = _current->previous;
		delete _current;
		_current = previous;
	}

	_current->offset -= _current->data[_current->offset - 1];
}
