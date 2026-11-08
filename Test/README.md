# Video Test plugin

Build with `make -f Makefile`, then run `Appeal.exe Test/Video` from the Appeal root. This plugin echoes **every decoder export call**, including lifecycle and metadata/frame/audio requests. It returns deterministic diagnostic values without accessing files, opening windows, or producing audio. The module's own playlist/texture routing is separate from decoder exports.

To start a real plugin, copy this folder next to Test, rename it, and edit each function in `src/main.cpp`. The Makefile uses the folder name for the output DLL/SO. Implement the complete [plugin SDK](../src/Video_Plugin_API.h), keeping media buffers owned by the plugin until the next call for that clip. Keep Constructor/Destructor cleanup and compile with Appeal's compiler ABI. Refer to the [FFmpeg implementation](../Appeal_FFmpeg_Video/src/main.cpp) for a working decoder.
