The source code requires a library called "Minhook", which is easy to find.
If it doesn't compile in Debug mode, stick to Release mode.

Additionally, have in mind that the soruce code uses a modified version of SKSE64 2.0.12.
The only changes in the SKSE64 2.0.12 is what I call "Universal DLL Project".
The Universal DLL project consists of two files (RelocationEx.cpp, RelocationEx.h) which add variants of RelocAddr, RelocPtr, and DEFINE_MEMBER_FN that accept signatures, as opposed to just addresses.
Several of the SKSE64 definitions have been changed to use these new types instead.
But it is not complete yet, look at "Universal DLL Project Status" to see what is done and what not.

In any case, all SKSE64 defintions used by Improved Camera (and a bit more), are already covered in the current Universal DLL Project.