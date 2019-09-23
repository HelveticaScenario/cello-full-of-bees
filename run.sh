#! /bin/bash
node . ./res/Riff.svg Riff ./src/Riff.cpp && \
node . ./res/PolyArp.svg Polyarp ./src/PolyArp.cpp && \
node . ./res/TappableClockSource.svg TappableClockSource ./src/TappableClockSource.cpp && \
make && make install && /Applications/Rack.app/Contents/MacOS/Rack