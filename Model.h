// Model.h

#ifndef MY_MODEL
#define MY_MODEL

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <boost/variant.hpp>
#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

#include <stdio.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;
///////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
public:
    // function prototypes ////////////////////////////////////////////////////////////////////////
    Model();
    
    vector< map<string, boost::variant<int, string>> > getCameras();
    void storeVolumeData(int camera_id, int volume_size);
    vector<boost::variant<int, string>> getVolumeByID(int camera_id);
    float getPercentage(int camera_id, string date_time, int volume_size);

};

#endif    // MY_MODEL
