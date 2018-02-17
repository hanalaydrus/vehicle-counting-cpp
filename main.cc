/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "boost/variant.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpc++/grpc++.h>

#include "volumeContract.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using volumeContract::HelloRequest;
using volumeContract::HelloReply;
using volumeContract::Greeter;

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include "Blob.h"
#include "Model.h" 
// #define SHOW_STEPS            // un-comment or comment this line to show steps or not

using namespace cv;
using namespace std;

// global variables ///////////////////////////////////////////////////////////////////////////////
const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_YELLOW = Scalar(0.0, 255.0, 255.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 200.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs);
void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex);
void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs);
double distanceBetweenPoints(Point point1, Point point2);
void drawAndShowContours(Size imageSize, vector<vector<Point> > contours, string strImageName);
void drawAndShowContours(Size imageSize, vector<Blob> blobs, string strImageName);
bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount);
void drawBlobInfoOnImage(vector<Blob> &blobs, Mat &imgFrame2Copy);
void drawCarCountOnImage(int &carCount, Mat &imgFrame2Copy);
void RunServer();

void RunService (int camera_id, string url, int x0, int y0, int x1, int y1) {
    Model model;

    Mat image, imgFrame1, imgFrame2;
    
    string carCountString, prevCarCountString;

    vector<Blob> blobs;

    int carCount = 0;

    // Crossing Line Configuration
    Point crossingLine[2];

    VideoCapture cap(url);

    cap >> image;

    imgFrame1 = image;
    imgFrame2 = image;

    int intHorizontalLinePosition = y0;

    crossingLine[0].x = x0;
    crossingLine[0].y = y0;
    
    crossingLine[1].x = x1;
    crossingLine[1].y = y1;

    char chCheckForEscKey = 0;

    bool blnFirstFrame = true;

    int frameCount = 2;

    // while (cap.isOpened() && chCheckForEscKey != 27) {
    for (;;) {
        model.storeVolumeData(camera_id, carCount);
        if (image.empty())
		{
            cout << "Input image empty get frame" << endl;
            cap.open(url);
            
            cap >> image;
            
            imgFrame1 = image;
            imgFrame2 = image;

			continue;
        }
        
        vector<Blob> currentFrameBlobs;

        Mat imgFrame1Copy = imgFrame1.clone();
        Mat imgFrame2Copy = imgFrame2.clone();

        Mat imgDifference;
        Mat imgThresh;

        cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
        cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

        GaussianBlur(imgFrame1Copy, imgFrame1Copy, Size(5, 5), 0);
        GaussianBlur(imgFrame2Copy, imgFrame2Copy, Size(5, 5), 0);

        absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

        threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);

        // imshow("imgThresh", imgThresh);

        Mat structuringElement3x3 = getStructuringElement(MORPH_RECT, Size(3, 3));
        Mat structuringElement5x5 = getStructuringElement(MORPH_RECT, Size(5, 5));
        Mat structuringElement7x7 = getStructuringElement(MORPH_RECT, Size(7, 7));
        Mat structuringElement15x15 = getStructuringElement(MORPH_RECT, Size(15, 15));

        for (unsigned int i = 0; i < 2; i++) {
            dilate(imgThresh, imgThresh, structuringElement5x5);
            dilate(imgThresh, imgThresh, structuringElement5x5);
            erode(imgThresh, imgThresh, structuringElement5x5);
        }

        Mat imgThreshCopy = imgThresh.clone();

        vector<vector<Point> > contours;

        findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        drawAndShowContours(imgThresh.size(), contours, "imgContours");

        vector<vector<Point> > convexHulls(contours.size());

        for (unsigned int i = 0; i < contours.size(); i++) {
            convexHull(contours[i], convexHulls[i]);
        }

        drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");

        for (auto &convexHull : convexHulls) {
            Blob possibleBlob(convexHull);

            if (possibleBlob.currentBoundingRect.area() > 400 &&
                possibleBlob.dblCurrentAspectRatio > 0.2 &&
                possibleBlob.dblCurrentAspectRatio < 4.0 &&
                possibleBlob.currentBoundingRect.width > 30 &&
                possibleBlob.currentBoundingRect.height > 30 &&
                possibleBlob.dblCurrentDiagonalSize > 60.0 &&
                (contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {
                currentFrameBlobs.push_back(possibleBlob);
            }
        }

        drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs");

        if (blnFirstFrame == true) {
            for (auto &currentFrameBlob : currentFrameBlobs) {
                blobs.push_back(currentFrameBlob);
            }
        } else {
            matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
        }

        drawAndShowContours(imgThresh.size(), blobs, "imgBlobs");

        imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

        drawBlobInfoOnImage(blobs, imgFrame2Copy);

        bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition, carCount);

        if (blnAtLeastOneBlobCrossedTheLine == true) {
            line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
        } else {
            line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
        }

        drawCarCountOnImage(carCount, imgFrame2Copy);
        
        carCountString = to_string(carCount);
        cout << "Count : " << carCountString << endl;

        // imshow("imgFrame2Copy", imgFrame2Copy); // THIS IS MAIN SHOW

        //waitKey(0);                 // uncomment this line to go frame by frame for debugging

        // now we prepare for the next iteration

        currentFrameBlobs.clear();

        imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is
        
        cap >> image;

        imgFrame2 = image;

        blnFirstFrame = false;
        frameCount++;
        chCheckForEscKey = waitKey(1);
    }

    if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
        waitKey(0);                         // hold the windows open to allow the "end of video" message to show
    }
}

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext* context,
                    const HelloRequest* request,
                    ServerWriter<HelloReply>* writer) override {
        HelloReply r;
        Model model;
        for (;;){
            vector<boost::variant<int, string>> response = model.getVolumeByID(request->id());
            float percentage = model.getPercentage(request->id(), boost::get<string>(response[0]), boost::get<int>(response[1]));
			r.set_timestamp(boost::get<string>(response[0]));
            r.set_volume(boost::get<int>(response[1]));
            r.set_percentage(50);
			writer->Write(r);
        }
        
        return Status::OK;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
    // note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows
    Model model;
    vector< map<string, boost::variant<int, string>> > cameras = model.getCameras();
    vector<int> index;

    for (int i = 0; i < cameras.size(); ++i){
        if (!cameras[i].empty()) {
            index.push_back(i);
        }
    }

    thread tRunService[index.size()];
    
    for (int i = 0; i < index.size(); ++i){
        tRunService[i] = thread (
            RunService, 
            index[i], 
            boost::get<string>(cameras[index[i]]["url"]),
            boost::get<int>(cameras[index[i]]["x0"]),
            boost::get<int>(cameras[index[i]]["y0"]),
            boost::get<int>(cameras[index[i]]["x1"]),
            boost::get<int>(cameras[index[i]]["y1"])
        );
    }

    thread tRunServer (RunServer);

    for (int i = 0; i < cameras.size(); ++i){
		tRunService[i].join();
    }
    tRunServer.join();

    return(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////// 
void RunServer() {
    string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;
  
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << endl;
  
    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs) {

    for (auto &existingBlob : existingBlobs) {

        existingBlob.blnCurrentMatchFoundOrNewBlob = false;

        existingBlob.predictNextPosition();
    }

    for (auto &currentFrameBlob : currentFrameBlobs) {

        int intIndexOfLeastDistance = 0;
        double dblLeastDistance = 100000.0;

        for (unsigned int i = 0; i < existingBlobs.size(); i++) {

            if (existingBlobs[i].blnStillBeingTracked == true) {

                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

                if (dblDistance < dblLeastDistance) {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;
                }
            }
        }

        if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.5) {
            addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
        }
        else {
            addNewBlob(currentFrameBlob, existingBlobs);
        }

    }

    for (auto &existingBlob : existingBlobs) {

        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {
            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
        }

        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
            existingBlob.blnStillBeingTracked = false;
        }

    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex) {

    existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

    existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

    existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
    existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

    existingBlobs[intIndex].blnStillBeingTracked = true;
    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs) {

    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;

    existingBlobs.push_back(currentFrameBlob);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(Point point1, Point point2) {

    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);

    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////
void drawAndShowContours(Size imageSize, vector<vector<Point> > contours, string strImageName) {
    Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    drawContours(image, contours, -1, SCALAR_WHITE, -1);

    // imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(Size imageSize, vector<Blob> blobs, string strImageName) {

    Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    vector<vector<Point> > contours;

    for (auto &blob : blobs) {
        if (blob.blnStillBeingTracked == true) {
            contours.push_back(blob.currentContour);
        }
    }

    drawContours(image, contours, -1, SCALAR_WHITE, -1);

    // imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount) {
    bool blnAtLeastOneBlobCrossedTheLine = false;

    for (auto blob : blobs) {

        if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2) {
            int prevFrameIndex = (int)blob.centerPositions.size() - 2;
            int currFrameIndex = (int)blob.centerPositions.size() - 1;

            if (blob.centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].y <= intHorizontalLinePosition) {
                carCount++;
                blnAtLeastOneBlobCrossedTheLine = true;
            }
        }

    }

    return blnAtLeastOneBlobCrossedTheLine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlobInfoOnImage(vector<Blob> &blobs, Mat &imgFrame2Copy) {

    for (unsigned int i = 0; i < blobs.size(); i++) {

        if (blobs[i].blnStillBeingTracked == true) {
            rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

            int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
            double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
            int intFontThickness = (int)round(dblFontScale * 1.0);

            putText(imgFrame2Copy, to_string(i), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCarCountOnImage(int &carCount, Mat &imgFrame2Copy) {

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
    int intFontThickness = (int)round(dblFontScale * 1.5);

    Size textSize = getTextSize(to_string(carCount), intFontFace, dblFontScale, intFontThickness, 0);

    Point ptTextBottomLeftPosition;

    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25);
    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);

    putText(imgFrame2Copy, to_string(carCount), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

