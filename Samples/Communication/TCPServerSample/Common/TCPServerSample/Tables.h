#pragma once

enum MonitorDBIndex
{
	ClientIncommingTable, //r:CommIncomingDB g:ClientComm
	ClientOutgoingTable,  //r:CommOutgoingDB g:ClientComm
	ServersClientIncommingTable, //r:CommIncomingDB g:ServerComm
	ServersClientOutgoingTable, //r:CommOutgoingDB g:ServerComm
	NumberOfDatabases
};

enum ProjectLogID
{
	NUM_OF_PROJECT_LOGS
};