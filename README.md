# Deca Proto
A Protobuf code generator for C++ projects running on micro-controllers.

## Objectives
Goals
- Make it easy to link from Arduino/PlatformIO projects

  Since Arduino's C++ stdlib lacks some libraries, it seems to be difficult to build the original Protobuf implementation from Google

- Provide richer interfaces compared to other light-weight Protobuf implementations

  For example, [Nanopb](https://github.com/nanopb/nanopb) generates structs from Proto messages.
  That's great but don't we want to rely on C++/STL container power?

NON-Goals
- Optimize the runtime heavily.

  If you want a super efficient runtime, choose the original Protobuf
