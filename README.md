# Deca Proto
A Protobuf code generator for C++ running on micro-controllers.

## Objectives
Goals
- Make it easy to compile on PlatformIO
  Since Arduino's C++ stdlib lacks some libraries, it seems to be difficult to build the original Protobuf from Google

NON-Goals
- Optimize the runtime heavily
  If you want a super efficient runtime, choose the original Protobuf
