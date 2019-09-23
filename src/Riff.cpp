#include "plugin.hpp"
#include "utils.hpp"

// scale mode intervals
// c c# d d# e f f# g g# a a# b
const bool SCALES[2][12] = {
    {true, true, true, true, true, true, true, true, true, true, true, true},     // Chromatic
    {true, false, true, false, true, true, false, true, false, true, false, true} // Major
};

struct Note
{
    int noteNum = 60;
    bool tie = false;
    bool on = false;
};

struct Riff : Module
{
    enum ParamIds
    {
        TAP_PARAM,
        MULT_DIV_PARAM,
        RESET_PARAM,

        ENUMS(STEP_SELECTOR_BUTTON_PARAM, 16),
        ENUMS(NOTE_SELECTOR_BUTTON_PARAM, 16),
        TIE_BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        TAP_INPUT,
        MULT_DIV_INPUT,
        RESET_INPUT,

        NUM_INPUTS
    };
    enum OutputIds
    {
        CLOCK_OUTPUT,
        MULT_DIV_OUTPUT,

        PITCH_OUT,
        GATE_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        TAP_LIGHT,
        RESET_LIGHT,

        ENUMS(STEP_SELECTOR_BUTTON_LIGHT, 16 * 3),
        ENUMS(PLAYHEAD_LIGHT, 16),
        ENUMS(STEP_TIE_LIGHT, 18),
        ENUMS(NOTE_SELECTOR_BUTTON_LIGHT, 16 * 3),
        ENUMS(PLAY_NOTE_LIGHT, 16),
        TIE_BUTTON_LIGHT,
        NUM_LIGHTS
    };

    Clock clock;

    int scale = 1;
    Note notes[16] = {};
    int selectedStep = 0;
    int playhead = 0;
    dsp::SchmittTrigger tapTrigger;
    dsp::SchmittTrigger resetInputTrigger;
    dsp::BooleanTrigger resetTrigger;
    // dsp::Timer clockResetTimer;
    dsp::Timer gateTimer;
    bool gate = false;
    bool pulseHigh = false;
    bool missedGate = false;
    // float getPitchInScale(float pitch)
    // {
    //     pitch = math::rescale(pitch, 0.f, 1.f, 0.f, 11.f);
    //     // int intPitch = (int)(pitch + 0.5 - (pitch < 0));
    //     float lowestDiff = 99.f;
    //     float lowestDiffPitch = 12.f;
    //     for (int i = 0; i < 12; i++)
    //     {
    //         if (SCALES[scale][i])
    //         {
    //             float p = (float)i;
    //             float diff = abs(p - pitch);
    //             if (diff < lowestDiff)
    //             {
    //                 lowestDiff = diff;
    //                 lowestDiffPitch = p;
    //             }
    //         }
    //     }
    //     if (lowestDiffPitch > 11.5f)
    //     {
    //         return 0.f;
    //     }

    //     return math::rescale(lowestDiffPitch, 0.f, 11.f, 0.f, 1.f);
    // }

    Riff()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(TAP_PARAM, 0.f, 2.f, 0.f, "Tap");
        configParam(MULT_DIV_PARAM, -9.f, 9.f, 0.f, "Numerator");
    }

    void process(const ProcessArgs &args) override
    {
        float sampleTime = args.sampleTime;

        resetInputTrigger.process(rescale(inputs[RESET_INPUT].getNormalVoltage(0.0f), 0.1f, 2.f, 0.f, 1.f));
        bool resetHigh = resetInputTrigger.isHigh() || params[RESET_PARAM].getValue() > 0.0f;
        lights[RESET_LIGHT].setBrightness(resetHigh > 0.0f ? 1.0f : 0.0f);
        bool reset = resetTrigger.process(resetHigh);

        float tapInput = inputs[TAP_INPUT].getNormalVoltage(params[TAP_PARAM].getValue());
        clock.process(inputs[MULT_DIV_INPUT].getNormalVoltage(params[MULT_DIV_PARAM].getValue()),
                      tapInput,
                      reset,
                      sampleTime,
                      [&](bool lightPulse, bool clockPulse, bool multDivClockPulse) {
                          lights[TAP_LIGHT].setSmoothBrightness(lightPulse ? 1.0f : 0.0f, sampleTime);
                          outputs[CLOCK_OUTPUT].setVoltage(clockPulse ? 10.0f : 0.0f);
                          outputs[MULT_DIV_OUTPUT].setVoltage(multDivClockPulse ? 10.0f : 0.0f);
                          if (!pulseHigh && multDivClockPulse)
                          {
                              pulseHigh = true;
                              gateTimer.reset();
                              playhead++;
                              playhead = playhead % 16;
                              if (gate)
                              {
                                  missedGate = true;
                              }
                          }
                          if (!multDivClockPulse)
                          {
                              pulseHigh = false;
                          }
                      });

        if (resetHigh)
        {
            gateTimer.reset();
            playhead = 0;
            if (gate)
            {
                missedGate = true;
            }
        }
        if (tapTrigger.process(rescale(tapInput, 0.1f, 2.f, 0.f, 1.f)))
        {
            gateTimer.reset();
            if (gate)
            {
                missedGate = true;
            }
        }
        if (missedGate)
        {
            if (gateTimer.time > 2e-3f)
            {
                gate = gateTimer.time < (clock.multDivDuration - 2e-3f);
                missedGate = false;
            }
            else
            {
                gate = false;
            }
        }
        else
        {
            gate = gateTimer.time < (clock.multDivDuration - 2e-3f);
        }

        bool selectedChanged = false;
        for (int i = 0; i < 16; i++)
        {
            float buttonVal = params[STEP_SELECTOR_BUTTON_PARAM + i].getValue();
            if (buttonVal > 0.f)
            {
                selectedStep = i;
                selectedChanged = true;
                break;
            }
        }
        if (selectedChanged)
        {
            for (int i = 0; i < 16; i++)
            {
                if (i == selectedStep)
                {
                    setLightOn(STEP_SELECTOR_BUTTON_LIGHT + (i * 3));
                }
                else
                {
                    setLightOff(STEP_SELECTOR_BUTTON_LIGHT + (i * 3));
                }
            }
        }
        for (int i = 0; i < 16; i++)
        {
            if (i == playhead)
            {
                lights[PLAYHEAD_LIGHT + i].setBrightness(1.0f);
            }
            else
            {
                lights[PLAYHEAD_LIGHT + i].setBrightness(0.0f);
            }
        }

        gateTimer.process(args.sampleTime);
    }

    void setLightOff(int id)
    {
        setRedBrightness(id, 0);
        setGreenBrightness(id, 0);
        setBlueBrightness(id, 0);
    }

    void setLightOn(int id)
    {
        setRedBrightness(id, 1);
        setGreenBrightness(id, 1);
        setBlueBrightness(id, 1);
    }

    void setRedBrightness(int id, float brightness)
    {
        lights[id].setBrightness(brightness);
    }
    void setGreenBrightness(int id, float brightness)
    {
        lights[id + 1].setBrightness(brightness);
    }
    void setBlueBrightness(int id, float brightness)
    {
        lights[id + 2].setBrightness(brightness);
    }
};

struct RiffWidget : ModuleWidget
{
    RiffWidget(Riff *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Riff.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// GEN_START
		Vec CLOCK_IN_POS = Vec(315, 86);
		Vec CLOCK_OUT_POS = Vec(348, 86);
		Vec MULT_DIV_IN_POS = Vec(315, 192);
		Vec MULT_DIV_OUT_POS = Vec(348, 192);
		Vec MULT_DIV_PARAM_POS = Vec(330, 139);
		Vec NOTE_LIGHT_00_POS = Vec(27.5, 85.5);
		Vec NOTE_LIGHT_01_POS = Vec(56.5, 85.5);
		Vec NOTE_LIGHT_02_POS = Vec(85.5, 85.5);
		Vec NOTE_LIGHT_03_POS = Vec(114.5, 85.5);
		Vec NOTE_LIGHT_04_POS = Vec(143.5, 85.5);
		Vec NOTE_LIGHT_05_POS = Vec(172.5, 85.5);
		Vec NOTE_LIGHT_06_POS = Vec(201.5, 85.5);
		Vec NOTE_LIGHT_07_POS = Vec(230.5, 85.5);
		Vec NOTE_LIGHT_08_POS = Vec(27.5, 108.5);
		Vec NOTE_LIGHT_09_POS = Vec(56.5, 108.5);
		Vec NOTE_LIGHT_10_POS = Vec(85.5, 108.5);
		Vec NOTE_LIGHT_11_POS = Vec(114.5, 108.5);
		Vec NOTE_LIGHT_12_POS = Vec(143.5, 108.5);
		Vec NOTE_LIGHT_13_POS = Vec(172.5, 108.5);
		Vec NOTE_LIGHT_14_POS = Vec(201.5, 108.5);
		Vec NOTE_LIGHT_15_POS = Vec(230.5, 108.5);
		Vec RESET_IN_POS = Vec(348, 245);
		Vec RESET_PARAM_POS = Vec(315, 245);
		Vec STEP_LIGHT_00_POS = Vec(27.5, 30.5);
		Vec STEP_LIGHT_01_POS = Vec(56.5, 30.5);
		Vec STEP_LIGHT_02_POS = Vec(85.5, 30.5);
		Vec STEP_LIGHT_03_POS = Vec(114.5, 30.5);
		Vec STEP_LIGHT_04_POS = Vec(143.5, 30.5);
		Vec STEP_LIGHT_05_POS = Vec(172.5, 30.5);
		Vec STEP_LIGHT_06_POS = Vec(201.5, 30.5);
		Vec STEP_LIGHT_07_POS = Vec(230.5, 30.5);
		Vec STEP_LIGHT_08_POS = Vec(27.5, 53.5);
		Vec STEP_LIGHT_09_POS = Vec(56.5, 53.5);
		Vec STEP_LIGHT_10_POS = Vec(85.5, 53.5);
		Vec STEP_LIGHT_11_POS = Vec(114.5, 53.5);
		Vec STEP_LIGHT_12_POS = Vec(143.5, 53.5);
		Vec STEP_LIGHT_13_POS = Vec(172.5, 53.5);
		Vec STEP_LIGHT_14_POS = Vec(201.5, 53.5);
		Vec STEP_LIGHT_15_POS = Vec(230.5, 53.5);
		Vec TAP_PARAM_POS = Vec(330, 33);
		Vec TIE_LIGHT_00_POS = Vec(42, 31);
		Vec TIE_LIGHT_01_POS = Vec(71, 31);
		Vec TIE_LIGHT_02_POS = Vec(100, 31);
		Vec TIE_LIGHT_03_POS = Vec(129, 31);
		Vec TIE_LIGHT_04_POS = Vec(158, 31);
		Vec TIE_LIGHT_05_POS = Vec(187, 31);
		Vec TIE_LIGHT_06_POS = Vec(216, 31);
		Vec TIE_LIGHT_07_POS = Vec(245, 31);
		Vec TIE_LIGHT_08_POS = Vec(42, 54);
		Vec TIE_LIGHT_09_POS = Vec(71, 54);
		Vec TIE_LIGHT_10_POS = Vec(100, 54);
		Vec TIE_LIGHT_11_POS = Vec(129, 54);
		Vec TIE_LIGHT_12_POS = Vec(158, 54);
		Vec TIE_LIGHT_13_POS = Vec(187, 54);
		Vec TIE_LIGHT_14_POS = Vec(216, 54);
		Vec TIE_LIGHT_15_POS = Vec(245, 54);
		Vec TIE_LIGHT_16_POS = Vec(13, 31);
		Vec TIE_LIGHT_17_POS = Vec(13, 54);
		// GEN_END

        Vec NoteLightPos[16] = {
            NOTE_LIGHT_00_POS,
            NOTE_LIGHT_01_POS,
            NOTE_LIGHT_02_POS,
            NOTE_LIGHT_03_POS,
            NOTE_LIGHT_04_POS,
            NOTE_LIGHT_05_POS,
            NOTE_LIGHT_06_POS,
            NOTE_LIGHT_07_POS,
            NOTE_LIGHT_08_POS,
            NOTE_LIGHT_09_POS,
            NOTE_LIGHT_10_POS,
            NOTE_LIGHT_11_POS,
            NOTE_LIGHT_12_POS,
            NOTE_LIGHT_13_POS,
            NOTE_LIGHT_14_POS,
            NOTE_LIGHT_15_POS,
        };

        Vec StepLightPos[16] = {
            STEP_LIGHT_00_POS,
            STEP_LIGHT_01_POS,
            STEP_LIGHT_02_POS,
            STEP_LIGHT_03_POS,
            STEP_LIGHT_04_POS,
            STEP_LIGHT_05_POS,
            STEP_LIGHT_06_POS,
            STEP_LIGHT_07_POS,
            STEP_LIGHT_08_POS,
            STEP_LIGHT_09_POS,
            STEP_LIGHT_10_POS,
            STEP_LIGHT_11_POS,
            STEP_LIGHT_12_POS,
            STEP_LIGHT_13_POS,
            STEP_LIGHT_14_POS,
            STEP_LIGHT_15_POS,
        };

        Vec TieLightPos[16] = {
            TIE_LIGHT_00_POS,
            TIE_LIGHT_01_POS,
            TIE_LIGHT_02_POS,
            TIE_LIGHT_03_POS,
            TIE_LIGHT_04_POS,
            TIE_LIGHT_05_POS,
            TIE_LIGHT_06_POS,
            TIE_LIGHT_07_POS,
            TIE_LIGHT_08_POS,
            TIE_LIGHT_09_POS,
            TIE_LIGHT_10_POS,
            TIE_LIGHT_11_POS,
            TIE_LIGHT_12_POS,
            TIE_LIGHT_13_POS,
            TIE_LIGHT_14_POS,
            TIE_LIGHT_15_POS,
        };

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                int index = ((i * 8) + j);
                addParam(createParamCentered<LEDButton>(StepLightPos[index], module, Riff::STEP_SELECTOR_BUTTON_PARAM + index));
                addChild(createLightCentered<LargeLight<WhiteLight>>(StepLightPos[index], module, Riff::PLAYHEAD_LIGHT + index));
                addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(StepLightPos[index], module, Riff::STEP_SELECTOR_BUTTON_LIGHT + (index * 3)));
                addChild(createLightCentered<SmallLight<WhiteLight>>(TieLightPos[index], module, Riff::STEP_TIE_LIGHT + index));

                addParam(createParamCentered<LEDButton>(NoteLightPos[index], module, Riff::NOTE_SELECTOR_BUTTON_PARAM + index));
                addChild(createLightCentered<LargeLight<WhiteLight>>(NoteLightPos[index], module, Riff::PLAY_NOTE_LIGHT + index));
                addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(NoteLightPos[index], module, Riff::NOTE_SELECTOR_BUTTON_LIGHT + (index * 3)));
            }
        }
        addChild(createLightCentered<SmallLight<WhiteLight>>(TIE_LIGHT_16_POS, module, Riff::STEP_TIE_LIGHT + 16));
        addChild(createLightCentered<SmallLight<WhiteLight>>(TIE_LIGHT_17_POS, module, Riff::STEP_TIE_LIGHT + 17));

        addParam(createParamCentered<CKD6>(TAP_PARAM_POS, module, Riff::TAP_PARAM));
        addChild(createLightCentered<TapLight>(TAP_PARAM_POS, module, Riff::TAP_LIGHT));
        addInput(createInputCentered<PJ301MPort>(CLOCK_IN_POS, module, Riff::TAP_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(CLOCK_OUT_POS, module, Riff::CLOCK_OUTPUT));

        {
            auto knob = createParamCentered<RoundSmallBlackKnob>(MULT_DIV_PARAM_POS, module, Riff::MULT_DIV_PARAM);
            knob->snap = true;
            addParam(knob);
        }
        addInput(createInputCentered<PJ301MPort>(MULT_DIV_IN_POS, module, Riff::MULT_DIV_INPUT));
        addInput(createInputCentered<PJ301MPort>(RESET_IN_POS, module, Riff::RESET_INPUT));
        addParam(createParamCentered<LEDButton>(RESET_PARAM_POS, module, Riff::RESET_PARAM));
        addChild(createLightCentered<MediumLight<WhiteLight>>(RESET_PARAM_POS, module, Riff::RESET_LIGHT));

        addOutput(createOutputCentered<PJ301MPort>(MULT_DIV_OUT_POS, module, Riff::MULT_DIV_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(Vec(20.0f, 200.0f), module, Riff::GATE_OUT));
    }
};

Model *modelRiff = createModel<Riff, RiffWidget>("Riff");