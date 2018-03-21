#!/bin/bash
protoc -I ../protobuf/ --cpp_out=../src/got7/protobuf ../protobuf/feed.proto
