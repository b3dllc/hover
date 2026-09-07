# Hover! Decompilation

> [!NOTE]
> This project has utilized large language models (colloquially known as AI) to assist in decompiling functions. However, all code has been thoroughly human-verified and refined for quality, correctness, and accuracy.

This is a work-in-progress matching decompilation of *Microsoft Hover!* for Windows 95. 

## Status / Roadmap

All functions have been implemented/decompiled with ~90% byte matching. There may be minor bugs/inaccuracies present that are hidden in that non-matching remainder. These will be gradually resolved as the match improves.

Goal is to eventually port to portable frameworks like SDL and modernize (i.e. high resolution, 60FPS+, controller support, hardware acceleration).

## Compiling

*Hover!* was compiled internally at Microsoft with what appears to be a combination of **Microsoft Visual C++ 2.1** and **MFC 3.1**. It is recommended to use this build, however the codebase will probably be compatible with most versions of Microsoft Visual C++, particularly earlier versions.

A simple NMake Makefile has been provided for compilation with `nmake`. Additionally, a `build.sh` is provided for compiling with Linux (via Wine).

## Contributions

Contributions that improve matches or implement functions are welcome, however this repository is not very well set up for outside contributions at this time.

It is also not intended to be used for porting or modding - it is intended solely for matching decompilation. Instead, any modding and porting efforts should be handled in downstream forks.

## Forking

You are welcome to fork the repository and make changes, however you should be aware that, as mentioned in under "Status", there may be minor lingering decompilation inaccuracies that are in the process of being caught. You should prepare your fork to accept upstream changes as improvements continue to be made.