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
	cv::Mat frame, oldFrame, image;
	cv::CascadeClassifier cascade, nestedCascade;
	double scale = 1.0;

	//Use OpenCV facial classifiers
	//TODO: train new classifiers to work better with glasses
	nestedCascade.load("./resources/haarcascade_eye_tree_eyeglasses.xml");
	cascade.load("./resources/haarcascade_frontalface_default.xml");

	cv::namedWindow("Configurations");
	int bright_slider = 100;
	int contrast_slider = 10;

	cv::createTrackbar("Brightness", "Configurations", &bright_slider, 200);
	cv::createTrackbar("Contrast", "Configurations", &contrast_slider, 20);

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
			for (int y = 0; y < frame1.rows; y++) {
				for (int x = 0; x < frame1.cols; x++) {
					for (int c = 0; c < 3; c++) {
						double alpha = ((contrast_slider - 10) >= 0) ? double(contrast_slider - 9) / 2 : 1.0 / double(cv::abs(11 - contrast_slider));
						double beta = bright_slider - 100;
						auto newVal = cv::saturate_cast<uchar>(alpha*(frame1.at<cv::Vec3b>(y, x)[c]) + beta);
						frame1.at<cv::Vec3b>(y, x)[c] = newVal;
					}
				}
			}
			//For higher speed denoising, use this; it almost works
			//cv::GaussianBlur(frame1, frame1, cv::Size(3, 3), 0, 0);
			//For super fast computers, this may be nice but is too slow for use
			//cv::fastNlMeansDenoisingColored(frame1, frame1);
			std::vector<cv::Rect> faces, faces2;
			cv::Mat grey, smallImg;
			cv::cvtColor(frame1, grey, cv::COLOR_BGR2GRAY);
			double fx = 1.0 / scale;
			cv::resize(grey, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR);
			cv::equalizeHist(smallImg, smallImg);
			//Detect the faces
			cascade.detectMultiScale(smallImg, faces, 1.1, 2, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
			//Save old frame if necessary
			if (oldFrame.empty()) {
				cv::swap(oldFrame, smallImg);
				continue;
			}
			#ifdef OFM
				cv::UMat uflow;
				cv::Mat flow;
				cv::calcOpticalFlowFarneback(oldFrame, smallImg, uflow, 0.5, 3, 15, 3, 5, 1.2, 0);
				uflow.copyTo(flow);
			#endif

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
				double aspectRatio = double(r.width)/double(r.height);
				if (0.75 < aspectRatio && aspectRatio < 1.3)
				{
					center.x = cvRound((r.x + r.width*0.5)*scale);
					center.y = cvRound((r.y + r.height*0.5)*scale);
					radius = cvRound((r.width + r.height)*0.25*scale);
				}
				Face face = {center, radius, color, std::vector<Eye>()};

				if (nestedCascade.empty())
					continue;
				smallImgROI = smallImg(r);

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

			//Draw circles around faces and eyes
			for (std::size_t i = 0; i < faceList.size(); i++) {
				cv::circle(frame1, faceList[i].center, faceList[i].radius, faceList[i].color, 5);
				for (std::size_t j = 0; j < faceList[i].eyes.size(); j++) {
					cv::circle(frame1, faceList[i].eyes[j].center, faceList[i].eyes[j].radius, faceList[i].color, 3);
				}
			}

			#ifdef OFM
				for (int y = 0; y < frame1.rows; y += 16) {
					for (int x = 0; x < frame1.cols; x += 16)
					{
						//Get the flow values at each point
						const cv::Point2f & fxy = flow.at<cv::Point2f>(y, x);

						//Get the scalar value of the motion at the point
						float scalarFlow = sqrt(fxy.x * fxy.x + fxy.y * fxy.y);

						unsigned __int8 red = 255.0 / (1.0 + exp(-scalarFlow / 4 + 2));
						unsigned __int8 blue = 255 - red;

						//Draw the line pointing in the direction of motion
						cv::line(frame1, cv::Point(x, y), cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), cv::Scalar(blue, 0, red));

						//Draw the point that we're looking at
						cv::circle(frame1, cv::Point(x, y), 2, cv::Scalar(blue, 0, red), -1);
					}
				}
			#endif

			imshow("Video Feed", frame);
			imshow("Face Detection", frame1);
			cv::swap(oldFrame, frame1);

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

