#pragma once

#include <string>

class Base_List
{
public:
	bool Looped;
	int Nodes;
	int Current_Node;
	std::string Current_Name;
	virtual void Next() = 0;
	virtual void Back() = 0;
};
