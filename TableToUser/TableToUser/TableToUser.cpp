#include "pch.h"
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
//TODO: Add OpenCV headers
#include <math.h>

#define BUFFER_LENGTH 2048
#define DEFAULT_PORT "25565"

void serveFunction();
void processing();

volatile bool serveStop = false;
volatile unsigned short numClients = 0;
volatile bool serveFail = false;

//TODO volatile vec3 table to user vector

int main(){

	std::thread* serveThread;
	serveThread = new std::thread(&serveFunction);

	processing();

	serveThread->join();
}

void processing() {
	//TODO: OpenCV processing to update the 

}

void serveFunction() {

	//Create server socket
	WSADATA wsaData;
	int iResult;

	SOCKET listener = INVALID_SOCKET;
	SOCKET client = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	char recvBuffer[BUFFER_LENGTH];
	size_t recvbuflen = BUFFER_LENGTH;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult) {
		serveFail = true;
		std::cerr << "WSA Startup Failure!" << std::endl;
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult) {
		serveFail = true;
		std::cerr << "Addressing failure" << std::endl;
		WSACleanup();
		return;
	}

	client = accept(listener, NULL, NULL);
	if (client == INVALID_SOCKET) {
		serveFail = true;
		std::cerr << "Failure to bind to requisite socket" << std::endl;
		closesocket(listener);
		WSACleanup();
		return;
	}

	do {
		iResult = recv(client, recvBuffer, recvbuflen, 0);

		if (iResult < 0) {
			serveFail = true;
			std::cerr << "Failure to recv from socket" << std::endl;
			closesocket(client);
			WSACleanup();
			return;
		}

		//TODO: parse bytes (should indicate request type and are stored in recvBuffer)
		//if a well formatted API request is recv'd, then send well formatted API response back
		//if malformed request, respond with the correct HTTP code
		if (iResult==0)
		{
			//client disconnected
		}
		else if (strlen(recvBuffer)==0)
		{
			//message has no content
		}
		
		//TODO: send response


	} while (iResult && !serveStop);

	//Clean up the socket when done
	iResult = shutdown(client, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(client);
		WSACleanup();
		std::cerr << "Ffailure to shutdown socket" << std::endl;
		serveFail = true;
	}

	closesocket(client);
	WSACleanup();

}

