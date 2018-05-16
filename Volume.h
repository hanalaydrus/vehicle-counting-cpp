// volume.h
#ifndef MY_VOLUME
#define MY_VOLUME

#include "boost/variant.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <thread>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include "Blob.h"
#include "Model.h"

using namespace cv;
using namespace std;

class Volume {
public:
	Volume();
	static void runVolumeService(int camera_id, string url, int x0, int y0, int x1, int y1);
};

#endif    // VOLUME