#!/bin/bash -e

go build -C codegen/ -o ./protoc-gen-deca_cpp

rm -r tmp/
mkdir -p tmp/googproto

protoc --plugin=./codegen/protoc-gen-deca_cpp --deca_cpp_out=./tmp --proto_path=./ ./tests/*.proto
protoc --cpp_out=./tmp/googproto/ --proto_path=./ ./tests/*.proto

echo Generated sample .pb.h under decaproto/tmp/...
