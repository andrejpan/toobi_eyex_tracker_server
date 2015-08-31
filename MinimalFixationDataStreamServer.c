/*
* This is an example that demonstrates how to connect to the EyeX Engine and subscribe to the fixation data stream.
*
* Copyright 2013-2014 Tobii Technology AB. All rights reserved.
*
* Added a simple tcp/ip sending server, refernce
* https://msdn.microsoft.com/en-us/library/windows/desktop/ms737593%28v=vs.85%29.aspx
*/

//sockets
#undef UNICODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "eyex/EyeX.h"

// sockets
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

//threads
#include <process.h> /* _beginthread() */

#pragma comment (lib, "Tobii.EyeX.Client.lib")
//sockets
#pragma comment (lib, "Ws2_32.lib")

// sockets
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1234"

// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "Rainbow Dash";

// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;

// sockets
SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;
// 0 disconected, 1 connected
int client_state = 0;
//1 thread running, 0 thread kill 
int end_threat = 1;

void handle(void *pParam)
{
	// sockets
	WSADATA wsaData;
	int iResult;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	while (end_threat == 1)
	{ 
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			// _endthread given to terminate
			_endthread();
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM; // connection oriented TCP protocol
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		printf("Waiting for a client\n");
		ClientSocket = accept(ListenSocket, NULL, NULL);

		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		printf("Client accepted\n");
		// end sockets

		client_state = 1;

		while (client_state == 1)
			{ }
	
		if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			// _endthread given to terminate
			_endthread();
		}

		// cleanup
		closesocket(ClientSocket);
		closesocket(ListenSocket);
		WSACleanup();

		printf("Client has just discnected\n");
	}

	if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
	}

	// cleanup
	closesocket(ClientSocket);
	closesocket(ListenSocket);
	WSACleanup();

	printf("Threat is exiting!\n");
	// _endthread given to terminate
	_endthread();
}



/*
* Initializes g_hGlobalInteractorSnapshot with an interactor that has the Fixation Data behavior.
*/
BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_FIXATIONDATAPARAMS params = { TX_FIXATIONDATAMODE_SENSITIVE };
	BOOL success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		InteractorId,
		&g_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateFixationDataBehavior(hInteractor, &params) == TX_RESULT_OK;

	txReleaseObject(&hInteractor);

	return success;
}

/*
* Callback function invoked when a snapshot has been committed.
*/
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	assert(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}

/*
* Callback function invoked when the status of the connection to the EyeX Engine has changed.
*/
void TX_CALLCONVENTION OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	switch (connectionState) {
	case TX_CONNECTIONSTATE_CONNECTED: {
		BOOL success;
		printf("The connection state is now CONNECTED (We are connected to the EyeX Engine)\n");
		// commit the snapshot with the global interactor as soon as the connection to the engine is established.
		// (it cannot be done earlier because committing means "send to the engine".)
		success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
		if (!success) {
			printf("Failed to initialize the data stream.\n");
		}
		else
		{
			printf("Waiting for fixation data to start streaming...\n\n");
		}
	}
									   break;

	case TX_CONNECTIONSTATE_DISCONNECTED:
		printf("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
		printf("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
		printf("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		printf("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.\n");
		break;
	}
}

/*
* Handles an event from the fixation data stream.
*/
void OnFixationDataEvent(TX_HANDLE hFixationDataBehavior)
{

	TX_FIXATIONDATAEVENTPARAMS eventParams;
	TX_FIXATIONDATAEVENTTYPE eventType;
	char* eventDescription;

	//sockets
	char gazeXbuff[13];
	char gazeYbuff[13];
	char szBuff[35];

	if (txGetFixationDataEventParams(hFixationDataBehavior, &eventParams) == TX_RESULT_OK) {
		eventType = eventParams.EventType;

		eventDescription = (eventType == TX_FIXATIONDATAEVENTTYPE_DATA) ? "Data"
			: ((eventType == TX_FIXATIONDATAEVENTTYPE_END) ? "End"
				: "Begin");

		printf("\r                                                                      ");
		printf("\rFixation %s: (%.1f, %.1f) timestamp %.0f ms", eventDescription, eventParams.X, eventParams.Y, eventParams.Timestamp);

		// client is connected, we can start sending data
		if (client_state == 1)
		{
			//sockets
			_snprintf(gazeXbuff, sizeof(gazeXbuff), "%i", (int)eventParams.X);
			_snprintf(gazeYbuff, sizeof(gazeYbuff), "%i", (int)eventParams.Y);
			strcpy(szBuff, gazeXbuff);
			strcat(szBuff, " ");
			strcat(szBuff, gazeYbuff);
			//printf("%s\n", szBuff);

			// Send a buffer
			int iResult = send(ClientSocket, szBuff, (int)strlen(szBuff), 0);
			if (iResult == SOCKET_ERROR)
			{

				printf("send failed with error: %d\n", WSAGetLastError());
				// clossing client connection
				//close_client_socket();
				client_state = 0;
			}
			else
			{
				printf("\r                                                                      ");
				printf("\rBytes sent: %ld", iResult);
			}
		}
	}
	else
	{
		printf("Failed to interpret fixation data event packet.\n");
	}
}

/*
* Callback function invoked when an event has been received from the EyeX Engine.
*/
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

	txGetAsyncDataContent(hAsyncData, &hEvent);

	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));

	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_FIXATIONDATA) == TX_RESULT_OK) {
		OnFixationDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}

	// NOTE since this is a very simple application with a single interactor and a single data stream, 
	// our event handling code can be very simple too. A more complex application would typically have to 
	// check for multiple behaviors and route events based on interactor IDs.

	txReleaseObject(&hEvent);
}

/*
* Application entry point.
*/
int main(int argc, char* argv[])
{
	TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
	TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
	TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;
	BOOL success;

	uintptr_t thread;
	thread = _beginthread(handle, 0, NULL);
	if (thread == -1) {
		fprintf(stderr, "Couldn't create thread: %d\n", GetLastError());
		return 1;
	}

	// initialize and enable the context that is our link to the EyeX Engine.
	success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
	success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
	success &= InitializeGlobalInteractorSnapshot(hContext);
	success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged, NULL) == TX_RESULT_OK;
	success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
	success &= txEnableConnection(hContext) == TX_RESULT_OK;

	// let the events flow until a key is pressed.
	if (success) {
		printf("Initialization was successful.\n");
	}
	else {
		printf("Initialization failed.\n");
	}


	printf("Press any key to exit...\n");
	_getch();
	end_threat = 0;
	printf("\nExiting.\n");

	// disable and delete the context.
	txDisableConnection(hContext);
	txReleaseObject(&g_hGlobalInteractorSnapshot);
	success = txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE) == TX_RESULT_OK;
	success &= txReleaseContext(&hContext) == TX_RESULT_OK;
	success &= txUninitializeEyeX() == TX_RESULT_OK;
	if (!success) {
		printf("EyeX could not be shut down cleanly. Did you remember to release all handles?\n");
	}

	return 0;
}
