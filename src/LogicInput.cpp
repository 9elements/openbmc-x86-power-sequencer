#include <iostream>
#include "Logic.hpp"
#include "LogicInput.hpp"

using namespace std;

// GetLevel is called by Logic when gathering it's new state
bool LogicInput::GetLevel()
{
	return (this->input->GetLevel() ^ this->invert);
}

// Dependency returns the Unit attached to
Signal* LogicInput::Dependency()
{
	return this->input;
}

LogicInput::LogicInput(Signal* in, Logic* par, bool inv, int stable)
{
	this->input = in;
	this->parent = par;
	this->invert = inv;
	this->inputStableUsec = stable;
}

