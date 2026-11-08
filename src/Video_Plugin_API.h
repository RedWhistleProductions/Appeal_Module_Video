#pragma once
#include <cstdint>
#include <string>
// Borrowed buffers: valid until the next plugin call for the same clip.
struct Video_Frame { std::uint32_t Version=1; int Width=0,Height=0,Stride=0; const std::uint8_t *Pixels=nullptr; std::uint64_t Serial=0; double Seconds=0; };
struct Video_PCM { std::uint32_t Version=1; int Rate=48000,Channels=2,Frames=0; const std::int16_t *Samples=nullptr; double Seconds=0; };
// All functions below are required exports, including no-op diagnostic versions.
#ifndef APPEAL_VIDEO_BRIDGE
extern "C" {
void Constructor(); void Destructor(); void Debug(bool); void Set_Paths(std::string);
bool Load(std::string,std::string); bool Use(std::string); void Close(std::string); void Close_All();
void Play(); void Pause(); void Stop(); bool Seek(double); void Set_Current_Loop(bool); void Update();
std::string Info(std::string,std::string); std::string Last_Error();
bool Get_Frame(std::string,Video_Frame*); bool Get_Audio(std::string,Video_PCM*);
}
#endif
