// Model.h

#ifndef MY_MODEL
#define MY_MODEL

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "boost/variant.hpp"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////
class Model {
public:
    // function prototypes ////////////////////////////////////////////////////////////////////////
    Model();
    
    vector< map<string, boost::variant<int, string>> > getCameras();
    void storeData(int camera_id, int volume_size);
    vector<string> getVolumeByID(int camera_id);

};

#endif    // MY_MODEL
