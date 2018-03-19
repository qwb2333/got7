#!/bin/bash
protoc -I ../protobuf/ --cpp_out=../src/model/ ../protobuf/feed.proto
