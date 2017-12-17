build:
	protoc -I . --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` volume.proto
	protoc -I . --cpp_out=. volume.proto