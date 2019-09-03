#! /bin/bash
node . ./res/PolyArp.svg Polyarp ./src/PolyArp.cpp && \
node . ./res/TappableClockSource.svg TappableClockSource ./src/TappableClockSource.cpp && \
make && make install && /Applications/Rack.app/Contents/MacOS/Rack