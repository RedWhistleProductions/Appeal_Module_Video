# Appeal Video module

Music-style video playback through interchangeable decoder plugins. Local projects:

- `Appeal_Module_Video`: script commands, resource names, playlists, Graphics/Audio routing.
- `Test`: echoing diagnostic and copyable plugin template; no media dependencies.
- `Appeal_FFmpeg_Video`: working FFmpeg file decoder; RGBA pictures and 48 kHz stereo PCM.

See [the complete Basic/Advanced API reference](Docs/Video.html).

## Run

```text
Appeal.exe Test/Video
Appeal.exe Examples/Video/Play_Video
Appeal.exe Examples/Video/Video_On_Surface
```

The sample is a generated three-second test pattern and tone, stored in `Resources/Video/Movies/Sample.mp4`. It is not third-party media. Standard `Setup/Config` loads the working plugin. An independent diagnostic needs only:

```text
Add_Dynamic_Module Video Test
Video Load Intro Example.mp4
Video Play
Video Update
Video Close_All
```

For real playback load Graphics, Audio, and Video (in that order), create a Graphics window, then use `Video Load`, `Video Play`, and one `Video Update` before each `Graphics Render`. Use `Video Draw Name X Y Width Height` for a cut-scene rectangle. For surfaces, `Video Bind_Texture Name Unique_Texture` then `Graphics Set_Texture Surface Unique_Texture`; the same frames update both ordinary 2D textures and UV-mapped mesh materials. Use `Audio Set_Volume_Percent`, `Audio Mute`, and `Audio UnMute` to control the soundtrack. Video Pause/Stop/Seek manage picture and soundtrack together.

Default movie and playlist paths are `Resources/Video/Movies` and `Resources/Video/Playlist`. Playlist lines use the same quoted-title convention as Audio: `"Title" Movie.mp4`. Load_Play_List appends; Clear_Play_List releases all clips. Relative movie paths resolve under Movies. Absolute file paths and direct FFmpeg-supported media URLs are accepted. A website's HTML page is not a media source; no site extraction/DRM support is provided.

## Build and install

Windows x64 uses the existing MinGW toolchain. Build module and Test without FFmpeg:

```text
make -C Resources/Modules/Appeal_Module_Video -f Makefile
make -C Resources/Modules/Appeal_Module_Video/Test -f Makefile
```

For the working plugin:

```text
python Resources/Modules/Appeal_Module_Video/Packaging/Install_FFmpeg.py
make -C Resources/Modules/Appeal_Module_Video/Appeal_FFmpeg_Video -f Makefile
```

The installer downloads BtbN's FFmpeg 8.1 LGPL shared SDK into the plugin's `.deps/ffmpeg`, checks GitHub's asset digest when supplied, records the URL and SHA-256 in `APPEAL_SOURCE.json`, and places the runtime DLLs beside Appeal.exe. Keep the SDK headers, import libraries, and runtime DLLs from the same archive. Rebuild the Audio and Graphics modules/plugins for the native video bridges. Their Test plugins expose diagnostic stubs, but only the real SDL2 backends display/mix output.

Linux Makefiles use pkg-config for libavformat, libavcodec, libavutil, libswscale, and libswresample, plus the project's SDL2 Graphics/Audio dependencies. Windows runtime is tested; Linux source/build paths have not been executed here.

## Plugin authors

Copy `Test` into a sibling directory and rename it. Its Makefile derives the DLL/SO name from the directory. Keep `src/Video_Plugin_API.h` available under the parent module. Implement every declared export, including Constructor/Destructor and frame/audio getters, using the same compiler/STL ABI as Appeal's existing string-based plugin APIs. Each Test export echoes its call; bridge-owned operations such as Draw/Bind_Texture route through the module and are not decoder hooks.

`Video_Frame` v1 borrows top-down RGBA8 pixels with a byte stride, monotonically increasing serial, dimensions, and presentation time in seconds. `Video_PCM` v1 borrows interleaved signed 16-bit stereo at 48 kHz, with frame count and presentation time. Buffers are owned by the decoder and valid until the next call affecting that clip. The module uploads/copies them immediately; no GPU handles or owned C++ containers cross these media bridge boundaries. Do not invoke Graphics uploads from decoder worker threads. Seek, Stop, Close, replacement Load, and destructor must clear owned decoder resources.

Module shutdown flushes output routes before unloading the decoder. The FFmpeg backend stages replacement loads so a failed file does not destroy a working clip, bounds lookahead queues, and uses RAII for codec/frame/packet/converter resources.

## Validation

```text
python Tests/Video_Module_Tests.py
python Tests/Video_Playback_Tests.py
```

Tests cover echo-export coverage, MP4/AVI/WebM, silent clips, repeated replacement/unload, failed-load preservation, real OpenGL pixels in a cut-scene and on a mesh, PCM consumption by Audio, volume/mute, pause, seek, looping, end-of-file, and release. Native audio tests use SDL's dummy device; they verify mixing/timeline activity rather than speaker output.

## FFmpeg redistribution

The plugin's own source uses this repository's MIT license. FFmpeg is a separate dependency; preserve its bundled license and configuration and satisfy the licenses of the exact binaries you ship. The installed LGPL shared archive, its LICENSE.txt, source URL, and SHA-256 are retained under `.deps/ffmpeg`; include those notices and the required matching FFmpeg source/relinking information in distributions. See https://ffmpeg.org/legal.html and https://github.com/BtbN/FFmpeg-Builds.
