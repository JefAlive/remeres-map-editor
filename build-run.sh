#!/bin/bash

cmake --build --preset linux-release -j 4
cp ./build/linux-release/bin/canary-map-editor canary-map-editor
./canary-map-editor