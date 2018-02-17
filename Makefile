build:
	protoc -I . --cpp_out=. volumeContract.proto
	g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o volumeContract.pb.o volumeContract.pb.cc
	protoc -I . --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` volumeContract.proto
	g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o volumeContract.grpc.pb.o volumeContract.grpc.pb.cc
	g++ -std=c++11 `pkg-config --cflags protobuf grpc opencv`  -c -o main.o main.cc
	g++ -std=c++11 `pkg-config --cflags opencv`  -c -o Blob.o Blob.cc
	g++ -std=c++11 -c -o Model.o Model.cc
	g++ volumeContract.pb.o volumeContract.grpc.pb.o main.o Blob.o Model.o -I/usr/inlude/cppconn -L/usr/lib -lmysqlcppconn -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc opencv` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -o main

run:
	./main

clean:
	rm -f *.o *.pb.cc *.pb.h main