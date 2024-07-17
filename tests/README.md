# A package for integration tests
This directory is for integration tests.
Tests under this directory would do

1. Run protoc to generate decaproto messages
1. Run tests on these generated messages

If you are looking for unit tests for runtime libraries running w/o the code generator
see //decaproto/runtime/tests
