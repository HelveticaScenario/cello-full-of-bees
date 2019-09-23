#include "utils.hpp"

void Clock::process(
    float multDivInput,
    float tapInput,
    bool resetHigh,
    float sampleTime,
    std::function<void(bool, bool, bool)> outputFunc)
{
    multDiv = (int)rint(clamp(multDivInput, -9.0, 9.0));
    bool clock = tapTrigger.process(rescale(tapInput, 0.1f, 2.f, 0.f, 1.f));
    if (clock)
    {
        if (tapping)
        {
            float time = timer.process(sampleTime);
            ring[tap] = time;
            tap++;
            if (tap >= 4)
            {
                hasEnoughTaps = true;
                tap = 0;
            }
            if (hasEnoughTaps)
            {
                duration = (ring[0] + ring[1] + ring[2] + ring[3]) / 4.0f;
            }
        }
        else
        {
            tapping = true;
        }
        timer.reset();
        time = duration;
        float multDuration = duration / (float)MULT_LCM;
        multTime = multDuration;
    }
    else if (tapping)
    {
        float time = timer.process(sampleTime);
        if (time > maxTapDistance)
        {
            timer.reset();
            tapping = false;
            hasEnoughTaps = false;
        }
    }

    if (resetHigh)
    {
        fullBeats = 0;
        multBeats = 0;
        timer.reset();
        tapping = false;
        hasEnoughTaps = false;
        time = duration;
        float multDuration = duration / (float)MULT_LCM;
        multTime = multDuration;
    }

    while (time >= duration)
    {
        time -= duration;
        clockPulse.trigger(1e-3f);
        lightPulse.trigger(0.1);
        if (multDiv == 0)
        {
            multDivClockPulse.trigger(1e-3f);
        }
        else if (multDiv < 0)
        {
            int div = multDivVals[multDiv + 9];
            if (fullBeats % div == 0)
            {
                multDivClockPulse.trigger(1e-3f);
            }
        }
        fullBeats++;
        fullBeats = fullBeats % DIV_LCM;
    }
    time += sampleTime;
    float multUnit = duration / (float)MULT_LCM;
    while (multTime >= multUnit)
    {
        multTime -= multUnit;
        if (multDiv > 0)
        {
            int mult = multDivVals[multDiv + 9];
            if (multBeats % (MULT_LCM / mult) == 0)
            {
                multDivClockPulse.trigger(1e-3f);
            }
        }
        multBeats++;
        multBeats = multBeats % MULT_LCM;
    }
    multTime += sampleTime;
    if (multDiv > 0)
    {
        multDivDuration = multUnit * multDivVals[multDiv + 9];
    }
    else
    {
        multDivDuration = duration * multDivVals[multDiv + 9];
    }
    outputFunc(lightPulse.process(sampleTime), clockPulse.process(sampleTime), multDivClockPulse.process(sampleTime));
};
