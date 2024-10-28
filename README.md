# GFS

> Game From Scratch

I am making this project during me watching
[Handmade Hero series](https://handmadehero.org/).

It's contain several demos. They use same library `gfs`, which places in `code/gfs`. All demos places inside `demos/`.

## Screenshots (badcraft Demo)

![Screenshot_00](/.github/screenshot_00.png)
![Screenshot_01](/.github/screenshot_01.png)
![Screenshot_02](/.github/screenshot_02.png)
![Screenshot_03](/.github/screenshot_03.png)

## How to build?

If you want compile only one demo, comment-out all unwanted demos in root `CMakeLists.txt`.

### Requirements

It is mainly supported on `Windows` OS. In fact, `Linux` (Wayland / Pipewire) will not be the target platform for this project.

Tools:

* Conan: version 2.x.
* CMake: version 3.20 (maybe it will build on older version, but I am lazy to check).
* MSVC: (Likewise, lazy to check which minimal version of the compiler you need).

DLLs (the should be on every Windows machine these days by default, since Windows XP):

* `Xinput.dll`
* `DirectSound.dll`

### Steps

1. Run build script.

```cmd
cmd /c build.bat
```

2. Done.
