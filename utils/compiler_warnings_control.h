#pragma once

#ifdef _WIN32

#define DISABLE_COMPILER_WARNINGS __pragma(warning(push, 0)) // Set /W0
#define RESTORE_COMPILER_WARNINGS __pragma(warning(pop))

#else

#define DISABLE_COMPILER_WARNINGS
#define RESTORE_COMPILER_WARNINGS

#endif

