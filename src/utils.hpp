#pragma once
#include <rack.hpp>

using namespace rack;

// LCM of 2,3,4,5,6,7,8,16,32
const int DIV_LCM = 3360;
// LCM of 2,3,4,5,6,7,8,12,16
const int MULT_LCM = 1680;

static const int multDivVals[] = {
    32,
    16,
    8,
    7,
    6,
    5,
    4,
    3,
    2,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    12,
    16};

struct Clock
{
    dsp::SchmittTrigger tapTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::PulseGenerator lightPulse;
    dsp::PulseGenerator clockPulse;
    dsp::PulseGenerator multDivClockPulse;
    dsp::Timer timer;
    std::vector<float> ring = {0.5f, 0.5f, 0.5f, 0.5f};
    float time = 0.0f;
    float multTime = 0.0f;
    float duration = 0.5f;
    float maxTapDistance = 2.0f;
    bool tapping = false;
    bool hasEnoughTaps = false;
    int tap = 0;
    int multDiv = 0;
    int fullBeats = 0;
    int multBeats = 0;
    float multDivDuration;
    void process(float multDivInput,
                 float tapInput,
                 bool resetHigh,
                 float sampleTime,
                 std::function<void(bool, bool, bool)> outputFunc);
};

struct TapLight : YellowLight
{
    TapLight()
    {
        box.size = Vec(28 - 6, 28 - 6);
        bgColor = color::BLACK_TRANSPARENT;
    }
};