#include "pch.h"
#include <iostream>
#include <vector>
#include <thread>
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <math.h>

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
	#warning "OpenCV Headers not found, please check your VisualStudio Configuration"
#endif

#define BUFFER_LENGTH 2048
#define DEFAULT_PORT "25565"

#pragma comment(lib, "Ws2_32.lib")

void serveFunction();
void processing();

volatile bool serveStop = false;
volatile unsigned short numClients = 0;
volatile bool serveFail = false;
#ifdef NO_CV
	constexpr double[] lookAt = {0.0, 0.0, 0.0};
	constexpr double[] eye = {0.0, 0.0, 0.0};
#else
	volatile cv::Vec3d lookAt(0.0, 0.0, 0.0);
	volatile cv::Vec3d eye(0.0, 0.0, 0.0);
#endif

int main(){

	std::thread* serveThread;
	serveThread = new std::thread(&serveFunction);

	processing();

	serveThread->join();
}

void processing() {
	 
#ifndef NO_CV
	//Create the image capture variables
	cv::VideoCapture capture;
	cv::Mat frame, image;
	cv::CascadeClassifier cascade, nestedCascade;
	double scale = 1.0;

	//Use OpenCV facial classifiers
	nestedCascade.load("./resources/haarcascade_eye_tree_eyeglasses.xml");
	cascade.load("./resources/haarcascade_frontalcatface.xml");

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
			//Image preprocessing
			std::vector<cv::Rect> faces, faces2;
			cv::Mat grey, smallImg;
			cv::cvtColor(frame1, grey, cv::COLOR_BGR2GRAY);
			double fx = 1.0 / scale;
			cv::resize(grey, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR);
			cv::equalizeHist(smallImg, smallImg);
			//Detect the faces
			cascade.detectMultiScale(smallImg, faces, 1.1, 2, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
			for (std::size_t i = 0; i < faces.size(); i++) {
				cv::Rect r = faces[i];
				cv::Mat smallImgROI; 
				std::vector<cv::Rect> nestedObjects;
				cv::Point center;
				cv::Scalar color(255, 0, 0);
				int radius;

				double aspectRatio = double(r.width)/double(r.height);
				if (0.75 < aspectRatio && aspectRatio < 1.3)
				{
					center.x = cvRound((r.x + r.width*0.5)*scale);
					center.y = cvRound((r.y + r.height*0.5)*scale);
					radius = cvRound((r.width + r.height)*0.25*scale);
					circle(frame1, center, radius, color, 3, 8, 0);
				}
				else
					rectangle(frame1, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
						cvPoint(cvRound((r.x + r.width - 1)*scale),
							cvRound((r.y + r.height - 1)*scale)), color, 3, 8, 0);
				if (nestedCascade.empty())
					continue;
				smallImgROI = smallImg(r);

				// Detection of eyes int the input image 
				nestedCascade.detectMultiScale(smallImgROI, nestedObjects, 1.1, 2, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

				// Draw circles around eyes 
				for (size_t j = 0; j < nestedObjects.size(); j++)
				{
					cv::Rect nr = nestedObjects[j];
					center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
					center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
					radius = cvRound((nr.width + nr.height)*0.25*scale);
					circle(frame1, center, radius, color, 3, 8, 0);
				}
			}

			imshow("Video Feed", frame);
			imshow("Face Detection", frame1);

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

void serveFunction() {

	//Create server socket
	WSADATA wsaData;
	int iResult;

	SOCKET listener = INVALID_SOCKET;
	SOCKET client = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	char recvBuffer[BUFFER_LENGTH];
	int recvbuflen = BUFFER_LENGTH;

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

