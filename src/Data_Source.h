#pragma once

#include <string>

class Data_Source
{
public:
	virtual std::string Get_Data();
	void operator >>(std::string &Data);
	void operator >>(int &Data);
	void operator >>(float &Data);
	void operator >>(double &Data);
	void operator >>(bool &Data);
    virtual void Return_Value(std::string) {}
    virtual bool End_Of_Source() const { return false; }
};
