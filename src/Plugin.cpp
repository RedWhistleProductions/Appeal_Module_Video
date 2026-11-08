#include "Plugin.h"

#ifdef __linux__
void Plugin::Load(std::string File)
{
	if(handle != 0)
	{
		Unload();
	}

	handle = dlopen(File.c_str(), RTLD_LAZY);
	if(!handle)
	{
		fputs(dlerror(), stderr);
		exit(1);
	}

	Assign("Constructor", Constructor);
	Assign("Destructor", Destructor);
	if(Constructor != nullptr)
	{
		Constructor();
	}
}

void Plugin::Unload()
{
	if(handle != 0)
	{
		void (*Destroy)() = reinterpret_cast<void (*)()>(dlsym(handle, "Destructor"));
		if(Destroy != nullptr)
		{
			Destroy();
		}
		dlclose(handle);
		handle = 0;
	}
	Constructor = nullptr;
	Destructor = nullptr;
}

Plugin::~Plugin()
{
	Unload();
}
#endif

#ifdef __MINGW32__
void Plugin::Load(std::string File)
{
	if(handle != 0)
	{
		Unload();
	}

	std::wstring wFile(File.begin(), File.end());
	handle = LoadLibraryW(wFile.c_str());
	if(!handle)
	{
		std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
		exit(1);
	}

	Assign("Constructor", Constructor);
	Assign("Destructor", Destructor);
	if(Constructor != nullptr)
	{
		Constructor();
	}
}

void Plugin::Unload()
{
	if(handle != 0)
	{
		void (*Destroy)() = reinterpret_cast<void (*)()>(GetProcAddress(handle, "Destructor"));
		if(Destroy != nullptr)
		{
			Destroy();
		}
		FreeLibrary(handle);
		handle = 0;
	}
	Constructor = nullptr;
	Destructor = nullptr;
}

Plugin::~Plugin()
{
	Unload();
}
#endif
