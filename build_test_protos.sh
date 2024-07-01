#!/bin/bash -e

cd generator/

go build -o ./protoc-gen-deca_cpp

protoc --plugin=./protoc-gen-deca_cpp --deca_cpp_out=../tmp --proto_path=../tests ../tests/*.proto
protoc --cpp_out=../tmp/googproto/ --proto_path=../tests ../tests/*.proto

echo Generated sample .pb.h under decaproto/tmp/...
