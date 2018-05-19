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
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <grpc++/grpc++.h>
#include "boost/variant.hpp"
#include "volumeContract.grpc.pb.h"

#include "Model.h" 
#include "Volume.h"

using namespace std::chrono;
using namespace cv;
using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using volumeContract::HelloRequest;
using volumeContract::HelloReply;
using volumeContract::Greeter;

/////////////////////////////////////////////////////////////////////////////////

static const char alphanum[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

char genRandom()
{

    return alphanum[rand() % stringLength];
}

int conccurrent = 0;

string printTime(){
    milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    return to_string(ms.count());
}

/////////////////////////////////////////////////////////////////////////////////

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext* context,
                    const HelloRequest* request,
                    ServerWriter<HelloReply>* writer) override {

        vector< vector<boost::variant<int, string>> > log;

        srand(time(0));
        string Str = to_string(request->id());
        for(unsigned int i = 0; i < 3; ++i)
        {
            Str += genRandom();

        }
        conccurrent++;

        // handle request
        HelloReply r;
        Model model;
        vector<boost::variant<int, string>> response;
        float percentage;

        cout << "start process" << endl;

        for (int i = 0; i < 1000; ++i) {
            // cout << "stream " << i << endl;
            vector<boost::variant<int, string>> logs;
            response = model.getVolumeByID(request->id());
            percentage = model.getPercentage(request->id(), boost::get<string>(response[0]), boost::get<int>(response[1]));
            r.set_timestamp(boost::get<string>(response[0]));
            // cout << "count: " << boost::get<int>(response[1]) << ", camera_id: "  << request->id() << endl;
            r.set_volume(boost::get<int>(response[1]));
            // cout << "percentage: " << percentage << endl;
            r.set_percentage(percentage);
            writer->Write(r);
            logs.push_back(Str);
            logs.push_back(printTime());
            logs.push_back(conccurrent);
            
            log.push_back(logs);
            
            if (context->IsCancelled()){
                conccurrent--;
                break;
            }
        }
        model.logging(log);
        cout << "Finish check log cc: " << conccurrent << endl;
        conccurrent--;
        return Status::OK;
    }
};

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
            Volume::runVolumeService, 
            index[i], 
            boost::get<string>(cameras[index[i]]["url"]),
            boost::get<int>(cameras[index[i]]["x0"]),
            boost::get<int>(cameras[index[i]]["y0"]),
            boost::get<int>(cameras[index[i]]["x1"]),
            boost::get<int>(cameras[index[i]]["y1"])
        );
    }

    thread tRunServer (RunServer);

    for (int i = 0; i < index.size(); ++i){
		tRunService[i].join();
    }
    tRunServer.join();

    return(0);
}

