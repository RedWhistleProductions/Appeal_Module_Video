#include "../../src/Video_Plugin_API.h"
#include <iostream>
#include <map>
namespace {std::string Current;std::map<std::string,bool> Playing;bool Loop=false;double Position=0;}
extern "C" void Constructor(){std::cout<<"Constructor\n";}
extern "C" void Destructor(){std::cout<<"Destructor\n";Playing.clear();Current.clear();}
extern "C" void Debug(bool value){std::cout<<"Debug "<<value<<'\n';}
extern "C" void Set_Paths(std::string root){std::cout<<"Set_Paths "<<root<<'\n';}
extern "C" bool Load(std::string name,std::string file){std::cout<<"Load "<<name<<" "<<file<<'\n';Playing[name]=false;Current=name;return true;}
extern "C" bool Use(std::string name){std::cout<<"Use "<<name<<'\n';if(!Playing.count(name))return false;Current=name;return true;}
extern "C" void Close(std::string name){std::cout<<"Close "<<name<<'\n';Playing.erase(name);}
extern "C" void Close_All(){std::cout<<"Close_All\n";Playing.clear();Current.clear();}
extern "C" void Play(){std::cout<<"Play\n";Playing[Current]=true;}
extern "C" void Pause(){std::cout<<"Pause\n";Playing[Current]=false;}
extern "C" void Stop(){std::cout<<"Stop\n";Playing[Current]=false;Position=0;}
extern "C" bool Seek(double seconds){std::cout<<"Seek "<<seconds<<'\n';Position=seconds;return true;}
extern "C" void Set_Current_Loop(bool loop){std::cout<<"Set_Current_Loop "<<loop<<'\n';Loop=loop;}
extern "C" void Update(){std::cout<<"Update\n";}
extern "C" std::string Info(std::string name,std::string field){std::cout<<"Info "<<name<<" "<<field<<'\n';if(field=="Has_Audio")return "1";if(field=="Is_Playing")return Playing[name]?"1":"0";if(field=="Looping")return Loop?"1":"0";if(field=="Position")return std::to_string(Position);return "0";}
extern "C" std::string Last_Error(){std::cout<<"Last_Error\n";return "";}
extern "C" bool Get_Frame(std::string name,Video_Frame*){std::cout<<"Get_Frame "<<name<<'\n';return false;}
extern "C" bool Get_Audio(std::string name,Video_PCM*){std::cout<<"Get_Audio "<<name<<'\n';return false;}
