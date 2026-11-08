#include "Data_Source.h"

std::string Data_Source::Get_Data()
{
	return "";
}

void Data_Source::operator >>(std::string &Data)
{
	Data = Get_Data();
}

void Data_Source::operator >>(int &Data)
{
	Data = std::stoi(Get_Data());
}

void Data_Source::operator >>(float &Data)
{
	Data = std::stof(Get_Data());
}

void Data_Source::operator >>(double &Data)
{
	Data = std::stod(Get_Data());
}

void Data_Source::operator >>(bool &Data)
{
	Data = Get_Data() == "true";
}
