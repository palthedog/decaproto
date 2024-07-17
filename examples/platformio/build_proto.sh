ROOT=$(git rev-parse --show-toplevel)
PROJECT=examples/platformio/

cd $ROOT

# TODO: Run the command via PlatformIO build tool
echo 'Building .cc proto...'
go build -C codegen/ -o ./protoc-gen-deca_cpp
protoc --plugin=codegen/protoc-gen-deca_cpp --deca_cpp_out=${PROJECT}/src/proto/ --proto_path=${PROJECT}/proto/ ${PROJECT}/proto/*.proto
