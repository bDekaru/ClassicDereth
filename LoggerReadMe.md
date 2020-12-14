EasyLogging++ usage for GDLE

Source: https://github.com/muflihun/easyloggingpp

Config File: GDLELogging.conf
- As defined will create new log file for Server info, Server error, Deaths, and Debug data
- Paths and file name can be modified as needed
- Needs to be in same folder as server exe file

For logging
 - Server info (default file: server.log)
	SERVER_INFO << [data for info];
	When adding variables use following format: SERVER_INFO << [data] << variable1; 
	Logger will add spaces automatically between the [data] & variable in above example
 - Server error (default file: error.log)
	SERVER_ERROR << [data for error]
    Variable usage same as SERVER_INFO
 - Debug info (default file: debug.log)
	DEBUG_DATA << [debug data]
    Variable usage same as SERVER_INFO
 - Deaths (default file: deaths.log)
	DEATH_LOG << [Message of player name] << ItemsDroppedText
	Variable usage same as SERVER_INFO
	
Any cpp file that will log to these logs will require an additional #include
	#include "easylogging++.h"

Example lines added:
 - PhatAC.cpp: line 559 - SERVER_INFO << "Welcome to GDLEnhanced!";
 - Server.cpp: line 366 - SERVER_INFO << "Bound to port" << port << "!";
 - Player.cpp: line 642 - DEATH_LOG << DEATH_LOG << InqStringQuality(NAME_STRING, "") << "-" << text;

 TODO: Update all logging to use appropriate log files