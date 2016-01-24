void OnExit();

#include <stdio.h>  /* defines FILENAME_MAX */
#include <stdlib.h>
#include <dirent.h>

#ifdef WIN32
    #include <Windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
#define REGISTER_SHUTDOWN_EVENT SetConsoleCtrlHandler( OnConsoleClose, TRUE );
BOOL WINAPI OnConsoleClose( DWORD dwCtrlType ){ OnExit(); return FALSE; }
#else
    #define _LINUX_ // unterscheidung von MAC OS und anderen noch notwenig. Am besten check auf Ubuntu etc.
    #include <unistd.h>
    #define GetCurrentDir getcwd
#define REGISTER_SHUTDOWN_EVENT // TODO!!!
#endif

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <errno.h>
#include <time.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
