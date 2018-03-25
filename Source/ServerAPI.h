
#pragma once

#ifdef _WINDLL

#define DECLTYPE __declspec(dllexport)

extern "C"
{
	DECLTYPE int _IsStarted();
	DECLTYPE int _StartServer();
	DECLTYPE void _StopServer();
};

#endif