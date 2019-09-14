#include "plugin.hpp"

struct SweepSeq : Module
{
    enum ParamIds
    {
        STEP_1_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    SweepSeq()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override
    {
    }
};

struct StepSlider : SliderKnob
{
    // widget::FramebufferWidget *fb;
    LightWidget *light;
    StepSlider()
    {
        // fb = new widget::FramebufferWidget;
        // addChild(fb);

        light = new LightWidget();
        light->bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
        light->borderColor = nvgRGBA(0, 0, 0, 0x60);
        light->box.pos = Vec(0, 0);
        light->box.size = Vec(50, 50);
        // light->
        // fb->addChild(light);
        // fb->dirty = true;
        addChild(light);
        // math::Vec margin = math::Vec(3.5, 3.5);
        // maxHandlePos = math::Vec(-1, -2).plus(margin);
        // minHandlePos = math::Vec(-1, 87).plus(margin);
        // setBackgroundSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSlidePot.svg")));
        // setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/BefacoSlidePotHandle.svg")));
        // background->box.pos = margin;
        box.size = Vec(100, 100);
    }
};

struct SweepSeqWidget : ModuleWidget
{
    SweepSeqWidget(SweepSeq *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SweepSeq.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addParam(createParamCentered<StepSlider>(Vec(0, 0), module, SweepSeq::STEP_1_PARAM));
		// GEN_START

		// GEN_END
    }
};

Model *modelSweepSeq = createModel<SweepSeq, SweepSeqWidget>("SweepSeq");