#include "Plugin.h"
#include "Data_Source.h"
#include "Module_Data.h"
#include "Named_List.h"
#define APPEAL_VIDEO_BRIDGE
#include "Video_Plugin_API.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
namespace {
Plugin Backend;
Named_List<Module_Data> *Globals=nullptr;
std::string Selected,Root="Resources",Playlist_Path="Resources/Video/Playlist",Error;
struct Binding {std::string Texture;uint64_t Serial=0;double Position=0;bool Audio=true;};
std::map<std::string,Binding> Bindings;std::vector<std::string> Order;
bool Auto_Next=false;
#define VIDEO_FUNCTIONS(X) \
 X(Debug,void,(bool)) X(Set_Paths,void,(std::string)) X(Load,bool,(std::string,std::string)) \
 X(Use,bool,(std::string)) X(Close,void,(std::string)) X(Close_All,void,()) \
 X(Play,void,()) X(Pause,void,()) X(Stop,void,()) X(Seek,bool,(double)) X(Set_Current_Loop,void,(bool)) \
 X(Update,void,()) X(Info,std::string,(std::string,std::string)) X(Last_Error,std::string,()) \
 X(Get_Frame,bool,(std::string,Video_Frame*)) X(Get_Audio,bool,(std::string,Video_PCM*))
#define DECLARE(Name,Type,Args) Type (*P_##Name) Args=nullptr;
VIDEO_FUNCTIONS(DECLARE)
#undef DECLARE
void Publish(std::string name,std::string type,std::string value){if(!Globals)return;Module_Data data{"Video",name,type,value};const auto key="Video."+name;if(Globals->Find(key))Globals->Current->Value=data;else Globals->Add_Node(key,data);}
void Defaults(){
    Publish("Auto_Play_Next_Status","bool",Auto_Next?"1":"0");
    Publish("Current_Video","string","");Publish("Texture","string","");Publish("Info","string","");
    for(const auto *name:{"Loaded","Selected","Seeked","Updated","Drawn","Is_Playing","Has_Audio","Current_Looping"})Publish(name,"bool","0");
    Publish("Width","int","0");Publish("Height","int","0");Publish("Duration","double","0");Publish("Position","double","0");
}
void Result(Data_Source *data,std::string name,std::string type,std::string value){Publish(name,type,value);data->Return_Value(value);}
void Set_Error(std::string value){Error=std::move(value);Publish("Last_Error","string",Error);}
bool Boolean(const std::string &value){if(value=="1"||value=="True"||value=="true"||value=="On")return true;if(value=="0"||value=="False"||value=="false"||value=="Off")return false;throw std::runtime_error("Expected a boolean value");}
// Borrow loaded modules; Video never loads or owns Graphics/Audio DLLs.
template<class T> T Hook(const char *module,const char *symbol){
#ifdef _WIN32
    auto handle=GetModuleHandleA(module);return handle?reinterpret_cast<T>(GetProcAddress(handle,symbol)):nullptr;
#else
    std::string file=std::string("Resources/Modules/")+module+"/"+module+".so";
    auto handle=dlopen(file.c_str(),RTLD_LAZY|RTLD_NOLOAD);if(!handle)return nullptr;
    auto result=reinterpret_cast<T>(dlsym(handle,symbol));dlclose(handle);return result;
#endif
}
#ifdef _WIN32
const char *Graphics="Appeal_Module_Graphics.dll",*Audio="Appeal_Module_Audio.dll";
#else
const char *Graphics="Appeal_Module_Graphics",*Audio="Appeal_Module_Audio";
#endif
void Flush(const std::string &name){if(auto fn=Hook<void(*)(const char*)>(Audio,"Audio_Video_Flush"))fn(name.c_str());}
void Pause_Sound(const std::string &name,bool pause){if(auto fn=Hook<void(*)(const char*,bool)>(Audio,"Audio_Video_Pause"))fn(name.c_str(),pause);}
void Release(const std::string &name,const Binding &binding){Flush(name);if(auto fn=Hook<void(*)(const char*)>(Graphics,"Graphics_Video_Release"))fn(binding.Texture.c_str());}
void Need_Selected(){if(Selected.empty() || !Bindings.count(Selected))throw std::runtime_error("No video selected. Use Video Load or Use first.");}
bool Choose(const std::string &name){if(!Bindings.count(name) || !P_Use(name)){Set_Error("Unknown video: "+name);return false;}Selected=name;Publish("Current_Video","string",name);return true;}
bool Load_Clip(const std::string &name,const std::string &file){if(!P_Load(name,file)){Set_Error(P_Last_Error());return false;}auto old=Bindings.find(name);if(old!=Bindings.end())Release(name,old->second);else Order.push_back(name);Bindings[name]={"Video."+name};Selected=name;Publish("Current_Video","string",name);Set_Error("");return true;}
void Stop_Selected(){Need_Selected();P_Stop();Flush(Selected);Bindings[Selected].Serial=0;Bindings[Selected].Position=0;Publish("Is_Playing","bool","0");}
void Step(int direction){Need_Selected();auto i=std::find(Order.begin(),Order.end(),Selected);const int index=int(i-Order.begin());Stop_Selected();Choose(Order[(index+direction+int(Order.size()))%int(Order.size())]);P_Play();Pause_Sound(Selected,false);Publish("Is_Playing","bool","1");}
bool Update_Outputs(){Set_Error("");P_Update();bool ok=true;
    for(auto &entry:Bindings){const auto &name=entry.first;auto &binding=entry.second;
        const auto position=P_Info(name,"Position");const double seconds=position.empty()?0:std::stod(position);
        if(seconds+0.01<binding.Position || seconds>binding.Position+0.5)Flush(name);
        binding.Position=seconds;
        Video_Frame frame;
        if(P_Get_Frame(name,&frame) && frame.Serial!=binding.Serial){
            auto upload=Hook<bool(*)(const char*,int,int,int,const unsigned char*)>(Graphics,"Graphics_Video_Upload");
            if(frame.Version!=1 || !frame.Pixels || frame.Width<=0 || frame.Height<=0 || frame.Stride<frame.Width*4){Set_Error("Invalid RGBA frame from video plugin");ok=false;}
            else if(upload && upload(binding.Texture.c_str(),frame.Width,frame.Height,frame.Stride,frame.Pixels))binding.Serial=frame.Serial;
            // Headless decoding is allowed; creating a Graphics window later uploads the retained frame.
        }
        const bool playing=P_Info(name,"Is_Playing")=="1";Pause_Sound(name,!playing);
        if(name==Selected){Publish("Position","double",position);Publish("Is_Playing","bool",playing?"1":"0");}
        if(playing && P_Info(name,"Has_Audio")=="1"){
            auto queue=Hook<bool(*)(const char*,const int16_t*,int,int,int)>(Audio,"Audio_Video_Queue");
            Video_PCM pcm;int count=0;
            while(count++<64 && P_Get_Audio(name,&pcm)) {
                if(!binding.Audio || pcm.Seconds+double(pcm.Frames)/48000 < seconds-0.1) continue;
                if(pcm.Version!=1 || !queue || !queue(name.c_str(),pcm.Samples,pcm.Frames,pcm.Rate,pcm.Channels)) {
                    Set_Error("Audio rejected video PCM samples; load a compatible Audio plugin or disable Audio_Output");ok=false;break;
                }
            }
        }
    }
    if(Auto_Next && !Selected.empty() && P_Info(Selected,"Ended")=="1" && !Order.empty()) Step(1);
    const auto decoder_error=P_Last_Error();if(!decoder_error.empty()){Set_Error(decoder_error);ok=false;}return ok;
}
}
extern "C" void Init_Globals(std::string path,Named_List<Module_Data> *globals){
    Globals=globals;Backend.Load(path);
    void (*lifecycle)()=nullptr;
    Backend.Assign("Constructor",lifecycle);if(!lifecycle){Backend.Unload();throw std::runtime_error("Video plugin missing Constructor");}
    Backend.Assign("Destructor",lifecycle);if(!lifecycle){Backend.Unload();throw std::runtime_error("Video plugin missing Destructor");}
#define ASSIGN(Name,Type,Args) Backend.Assign(#Name,P_##Name);if(!P_##Name){Backend.Unload();throw std::runtime_error("Video plugin missing required export: " #Name);}
    VIDEO_FUNCTIONS(ASSIGN)
#undef ASSIGN
    P_Set_Paths(Root);Defaults();Publish("Last_Error","string","");
}
extern "C" void Init(std::string path){Init_Globals(path,nullptr);}
extern "C" void Shutdown(){for(auto &entry:Bindings)Release(entry.first,entry.second);Bindings.clear();Order.clear();Selected.clear();Backend.Unload();Globals=nullptr;
#define CLEAR(Name,Type,Args) P_##Name=nullptr;
    VIDEO_FUNCTIONS(CLEAR)
#undef CLEAR
}
extern "C" void Interpreter(Data_Source *data){std::string command;*data>>command;
    if(command=="Load" || command=="Load_Video"){std::string name,file;*data>>name;*data>>file;Result(data,"Loaded","bool",Load_Clip(name,file)?"1":"0");}
    else if(command=="Use"){std::string name;*data>>name;Result(data,"Selected","bool",Choose(name)?"1":"0");}
    else if(command=="Play" || command=="Pause" || command=="Stop"){
        Need_Selected();if(command=="Play"){P_Play();Pause_Sound(Selected,false);}if(command=="Pause"){P_Pause();Pause_Sound(Selected,true);}if(command=="Stop")Stop_Selected();
        Publish("Is_Playing","bool",P_Info(Selected,"Is_Playing"));
    }
    else if(command=="Play_Video"){std::string name;*data>>name;if(Choose(name)){P_Play();Pause_Sound(name,false);}}
    else if(command=="Play_Track"){int track;*data>>track;if(track<1 || track>int(Order.size()))throw std::runtime_error("Video track out of range");if(!Selected.empty())Stop_Selected();Choose(Order[track-1]);P_Play();Pause_Sound(Selected,false);}
    else if(command=="Next" || command=="Back")Step(command=="Next"?1:-1);
    else if(command=="Seek"){double seconds;*data>>seconds;Need_Selected();const bool success=P_Seek(seconds);if(success){Flush(Selected);Bindings[Selected].Serial=0;Bindings[Selected].Position=seconds;}else Set_Error(P_Last_Error());Result(data,"Seeked","bool",success?"1":"0");}
    else if(command=="Set_Current_Loop"){std::string loop;*data>>loop;Need_Selected();P_Set_Current_Loop(Boolean(loop));}
    else if(command=="Auto_Play_Next"){std::string value;*data>>value;Auto_Next=Boolean(value);Publish("Auto_Play_Next_Status","bool",Auto_Next?"1":"0");}
    else if(command=="Auto_Play_Next_Status")Result(data,"Auto_Play_Next_Status","bool",Auto_Next?"1":"0");
    else if(command=="Update")Result(data,"Updated","bool",Update_Outputs()?"1":"0");
    else if(command=="Bind_Texture"){std::string name,texture;*data>>name;*data>>texture;auto it=Bindings.find(name);if(it==Bindings.end() || texture.empty())throw std::runtime_error("Invalid video/texture name");if(it->second.Texture!=texture){Release(name,it->second);it->second.Texture=texture;it->second.Serial=0;}}
    else if(command=="Get_Texture"){std::string name;*data>>name;auto it=Bindings.find(name);Result(data,"Texture","string",it==Bindings.end()?"":it->second.Texture);}
    else if(command=="Draw"){std::string name;int x,y,w,h;*data>>name;*data>>x;*data>>y;*data>>w;*data>>h;auto it=Bindings.find(name);auto draw=Hook<bool(*)(const char*,int,int,int,int)>(Graphics,"Graphics_Video_Draw");const bool drawn=it!=Bindings.end() && draw && draw(it->second.Texture.c_str(),x,y,w,h);Result(data,"Drawn","bool",drawn?"1":"0");}
    else if(command=="Audio_Output"){std::string name,value;*data>>name;*data>>value;auto it=Bindings.find(name);if(it==Bindings.end())throw std::runtime_error("Unknown video");it->second.Audio=Boolean(value);if(!it->second.Audio)Flush(name);}
    else if(command=="Info"){std::string name,field;*data>>name;*data>>field;Result(data,"Info","string",P_Info(name,field));}
    else if(command=="Current_Video")Result(data,"Current_Video","string",Selected);
    else if(command=="Is_Playing" || command=="Duration" || command=="Position" || command=="Width" || command=="Height" || command=="Has_Audio" || command=="Current_Looping"){
        const std::string field=command=="Current_Looping"?"Looping":command;const auto value=Selected.empty()?"0":P_Info(Selected,field);
        Result(data,command,(command=="Is_Playing"||command=="Has_Audio"||command=="Current_Looping")?"bool":(command=="Width"||command=="Height")?"int":"double",value);
    }
    else if(command=="Last_Error")Result(data,"Last_Error","string",Error.empty()?P_Last_Error():Error);
    else if(command=="Close"){std::string name;*data>>name;auto it=Bindings.find(name);if(it!=Bindings.end()){Release(name,it->second);Bindings.erase(it);Order.erase(std::remove(Order.begin(),Order.end(),name),Order.end());}P_Close(name);if(Selected==name){Selected=Order.empty()?"":Order.front();if(!Selected.empty())P_Use(Selected);}Publish("Current_Video","string",Selected);}
    else if(command=="Close_All" || command=="Clear_Play_List"){for(auto &entry:Bindings)Release(entry.first,entry.second);Bindings.clear();Order.clear();Selected.clear();P_Close_All();Defaults();}
    else if(command=="Set_Paths"){*data>>Root;P_Set_Paths(Root);Playlist_Path=(std::filesystem::path(Root)/"Video/Playlist").u8string();}
    else if(command=="Set_Playlist_Path"){*data>>Playlist_Path;}
    else if(command=="Debug"){std::string value;*data>>value;P_Debug(Boolean(value));}
    else if(command=="Load_Play_List"){
        std::string file;*data>>file;std::ifstream input(std::filesystem::u8path(Playlist_Path)/std::filesystem::u8path(file));if(!input){Set_Error("Could not open video playlist");Result(data,"Loaded","bool","0");return;}
        std::string line;bool success=true;while(std::getline(input,line)){std::istringstream fields(line);fields>>std::ws;if(fields.peek()=='#' || fields.peek()=='/' || fields.eof())continue;std::string name,source;fields>>std::quoted(name);fields>>std::ws;std::getline(fields,source);if(!source.empty() && source.back()=='\r')source.pop_back();if(source.empty()){source=name;name=std::filesystem::path(source).stem().u8string();}if(!Load_Clip(name,source))success=false;}
        if(!Order.empty())Choose(Order.front());
        Result(data,"Loaded","bool",success?"1":"0");
    }
    else throw std::runtime_error("Unknown Video command: "+command);
}
