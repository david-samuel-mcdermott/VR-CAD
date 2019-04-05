#include "pch.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <math.h>
#include <tuple>
#include <time.h>
#include <strstream>
#include<tchar.h>

#pragma comment(lib,"Ws2_32.lib")

//disable POSIX Errors in MSVC++
#ifdef _MSC_VER
	#pragma warning(disable : 4996)
	#define _CRT_SECURE_NO_WARNINGS
#endif

//Allow compilation even if OpenCV is not installed for testing purposes
#if __has_include(<opencv2/opencv.hpp>)
	#include <opencv2/objdetect.hpp>
	#include <opencv2/highgui.hpp>
	#include <opencv2/imgproc.hpp>
	#include <opencv2/opencv.hpp>
#else
	#define NO_CV
	//#warning "OpenCV Headers not found, please check your VisualStudio Configuration"
#endif

#define BUFFER_LENGTH 2048
#define DEFAULT_PORT "25565"
#define HOST_NAME "127.0.0.1"

void serveFunction();
void processing();

volatile bool serveStop = false;
volatile unsigned short numClients = 0;
volatile bool serveFail = false;

#ifdef NO_CV
	//double[] lookAt = {0.0, 0.0, 0.0};
	//double[] eye = {0.0, 0.0, 0.0};
#else
	cv::Vec3d lookAt(0.0, 0.0, 0.0);
	cv::Vec3d eye(0.0, 0.0, 0.0);

	typedef struct Eye {
		cv::Point center;
		double radius;
	} Eye;

	typedef struct Face {
		cv::Point center;
		double radius;
		cv::Scalar color;
		std::vector<Eye> eyes;
	} Face;

#endif

int main(){

	std::thread* serveThread;
	serveThread = new std::thread(&serveFunction);
	srand(time(nullptr));

	processing();

	serveThread->join();
}

void processing() {
	 
#ifndef NO_CV
	std::vector < Face > faceList;

	//Create the image capture variables
	cv::VideoCapture capture;
	cv::Mat frame, image;
	cv::CascadeClassifier cascade, nestedCascade;
	double scale = 1.0;

	//Use OpenCV facial classifiers
	//TODO: train new classifiers to work better with glasses
	nestedCascade.load("./resources/haarcascade_eye_tree_eyeglasses.xml");
	cascade.load("./resources/haarcascade_frontalface_default.xml");

	cv::namedWindow("Configurations");
	int bright_slider = 100;
	int contrast_slider = 10;
	int denoise_slider = 3;

	cv::createTrackbar("Brightness", "Configurations", &bright_slider, 200);
	cv::createTrackbar("Contrast", "Configurations", &contrast_slider, 20);
	cv::createTrackbar("Denoising", "Configurations", &denoise_slider, 10);

	capture.open(0);
	if (capture.isOpened()) {

		std::cout << "Processing loop starting" << std::endl;
		for (;;) {
			//Extract the next frame
			capture >> frame;

			if (frame.empty()) {
				std::cout << "Empty frame; video disconnected" << std::endl;
				break;
			}
			cv::Mat frame1 = frame.clone();

			std::vector<cv::Rect> faces, faces2;
			cv::Mat grey, processFrame, vizFrame;
			cv::cvtColor(frame1, grey, cv::COLOR_BGR2GRAY);
			double fx = 1.0 / scale;
			cv::resize(grey, processFrame, cv::Size(), fx, fx, cv::INTER_LINEAR);
			for (int y = 0; y < processFrame.rows; y++) {
				for (int x = 0; x < processFrame.cols; x++) {
					double alpha = ((contrast_slider - 10) >= 0) ? double(contrast_slider - 9) / 2 : 1.0 / double(cv::abs(11 - contrast_slider));
					double beta = bright_slider - 100;
					uchar newVal = cv::saturate_cast<uchar>(alpha*(processFrame.at<uchar>(y, x)) + beta);
					processFrame.at<uchar>(y, x) = newVal;
				}
			}
			cv::equalizeHist(processFrame, processFrame);
			if (denoise_slider != 0) {
				cv::fastNlMeansDenoising(processFrame, processFrame, denoise_slider);
			}
			//Detect the faces
			cascade.detectMultiScale(processFrame, faces, 1.1, 2, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
			//Save old frame if necessary

			//For face in faces
			std::vector<Face> newFaces;
			for (std::size_t i = 0; i < faces.size(); i++) {
				cv::Rect r = faces[i];
				cv::Mat smallImgROI;
				std::vector<cv::Rect> nestedObjects;
				cv::Point center;
				cv::Scalar color(rand() % 255, rand() % 255, rand() % 255);
				int radius;

				//Mark faces
				double aspectRatio = double(r.width) / double(r.height);
				if (0.75 < aspectRatio && aspectRatio < 1.3)
				{
					center.x = cvRound((r.x + r.width*0.5)*scale);
					center.y = cvRound((r.y + r.height*0.5)*scale);
					radius = cvRound((r.width + r.height)*0.25*scale);
				}
				Face face = { center, radius, color, std::vector<Eye>() };

				if (nestedCascade.empty())
					continue;
				smallImgROI = processFrame(r);

				// Detection of eyes in the input image 
				nestedCascade.detectMultiScale(smallImgROI, nestedObjects, 1.1, 2, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

				// Identify Eyes
				for (size_t j = 0; j < nestedObjects.size(); j++)
				{
					cv::Rect nr = nestedObjects[j];
					center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
					center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
					radius = cvRound((nr.width + nr.height)*0.25*scale);
					Eye eye = { center, radius };
					face.eyes.push_back(eye);
				}
				newFaces.push_back(face);
			}

			//Interframe facial tracking - bipartite lists
			for (std::vector<Face>::iterator iter = faceList.begin(); iter != faceList.end(); iter++) {
				Face face = *iter;
				Face nearest;
				double distance = std::numeric_limits<double>::infinity();
				//Remove missing faces
				if (newFaces.size() < 1) {
					do {
						faceList.erase(iter);
					} while (iter != faceList.end());
					break;
				}

				//Find match for face
				for (std::vector<Face>::iterator iter2 = newFaces.begin(); iter2 != newFaces.end(); iter2++) {
					Face face2 = *iter2;
					double dist = cv::abs(face2.center.dot(face.center));
					if (dist < distance) {
						distance = dist;
						nearest = face2;
					}
				}
				nearest.color = face.color;
				*iter = nearest;
				//Remove found face
				for (std::vector<Face>::iterator iter2 = newFaces.begin(); iter2 != newFaces.end(); iter2++) {
					Face face2 = *iter2;
					if (face2.center == nearest.center) {
						newFaces.erase(iter2);
						break;
					}
				}
			}
			//Add new faces in
			for (std::vector<Face>::iterator iter = newFaces.begin(); iter != newFaces.end(); iter++) {
				faceList.push_back(*iter);
			}

			cv::cvtColor(processFrame, vizFrame, cv::COLOR_GRAY2BGR);

			//Draw circles around faces and eyes
			for (std::vector<Face>::iterator iter = faceList.begin(); iter != faceList.end(); iter++) {
				Face face = *iter;
				cv::circle(vizFrame, face.center, face.radius, face.color, 5);
				for (std::vector<Eye>::iterator iter2 = face.eyes.begin(); iter2 != face.eyes.end(); iter2++) {
					Eye eye = *iter2;
					cv::circle(vizFrame, eye.center, eye.radius, face.color, 3);
				}
			}

			//Set variables for other thread
			if (faceList.size() > 0) {
				eye[0] = faceList[0].center.x;
				eye[1] = faceList[0].center.y;
				lookAt[0] = eye[0];
				lookAt[1] = eye[1];
			}


			imshow("Face Detection", vizFrame);

			char c = (char)cv::waitKey(1);
			if (c == VK_ESCAPE) {
				std::cout << "Escape pressed, exiting" << std::endl;
				break;
			}
		}
	}
	serveStop = true;
#else
	std::cout << "No Vision Capability, halting thread.  Server still will function\n" << std::endl;
#endif

}

int sendData(int sckt, void *data, int dataLength) {
	char *msgData = (char *)data;
	int bytesSent;

	//call send in a loop until proper bytes of data have been sent to client
	while (dataLength > 0) {
		bytesSent = send(sckt, msgData, dataLength, 0);
		if (bytesSent == -1) {
			return -1;
		}
		msgData += bytesSent;
		dataLength -= bytesSent;
	}

	return 0;
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

	iResult = getaddrinfo(HOST_NAME, DEFAULT_PORT, &hints, &result);
	if (iResult) {
		serveFail = true;
		std::cerr << "Addressing failure" << std::endl;
		WSACleanup();
		return;
	}
	// Create a SOCKET for connecting to server
	listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listener == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iResult = bind(listener, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listener);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	iResult = listen(listener, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listener);
		WSACleanup();
		return;
	}

	client = accept(listener, NULL, NULL);
	if (client == INVALID_SOCKET) {
		std::cout << WSAGetLastError() << std::endl;
		serveFail = true;
		std::cerr << "Failure to bind to requisite socket" << std::endl;
		closesocket(listener);
		WSACleanup();
		return;
	}

	do {
		iResult = recv(client, recvBuffer, recvbuflen, 0);
		std::cout << recvBuffer << std::endl;
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

		std::strstream wsss;
		wsss << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: text/html; charset=utf-8 \r\n"
			<< "Content-Length: " << sizeof(L"this shit working") << "\r\n"
			<< L"this shit working"
			<< "\r\n\r\n";
		//send headers
		std::string headers = wsss.str();
		int res = sendData(client, (void *)headers.c_str(), headers.size());
		if (res == -1) {
			//Error with sending the header
		}
		res = sendData(client, recvBuffer, recvbuflen);
		if (res == -1) {
			//error sending response data
		}

		if (iResult == 0)
		{
			//client disconnected
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
