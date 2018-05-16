build:
	protoc -I . --cpp_out=. volumeContract.proto
	g++ -std=c++14 `pkg-config --cflags --libs protobuf grpc`  -c -o volumeContract.pb.o volumeContract.pb.cc
	protoc -I . --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` volumeContract.proto
	g++ -std=c++14 `pkg-config --cflags --libs protobuf grpc`  -c -o volumeContract.grpc.pb.o volumeContract.grpc.pb.cc
	g++ -std=c++14 `pkg-config --cflags --libs protobuf grpc opencv`  -c -o main.o main.cc
	g++ -std=c++14 `pkg-config --cflags --libs protobuf grpc opencv`  -c -o Volume.o Volume.cc
	g++ -std=c++14 `pkg-config --cflags opencv`  -c -o Blob.o Blob.cc
	g++ -std=c++14 -c -o Model.o Model.cc
	g++ volumeContract.pb.o volumeContract.grpc.pb.o main.o Volume.o Blob.o Model.o -I/usr/inlude/cppconn -L/usr/lib -lmysqlcppconn -lcurl -L/usr/local/lib `pkg-config --cflags --libs protobuf grpc++ grpc opencv` -Wl,--no-as-needed -lgrpc++_reflection -ldl -o main

run:
	./main

clean:
	rm -f *.o *.pb.cc *.pb.h main

build_db_image:
	docker build -t asia.gcr.io/tugas-akhir-hana/db-volume:latest ./mariadb/

# if no trigger at google container registry
# build_service_image:
# 	docker build -t asia.gcr.io/tugas-akhir-hana/volume-of-vehicle:latest .

push_gcp_image:
	docker push asia.gcr.io/tugas-akhir-hana/db-volume:latest
	# docker push asia.gcr.io/tugas-akhir-hana/volume-of-vehicle:latest