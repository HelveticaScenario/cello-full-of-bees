#include "plugin.hpp"

struct Polyarp : Module
{
	enum ParamIds
	{
		TYPE_PARAM_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		TYPE_IN_INPUT,
		SIG_IN_INPUT,
		CLOCK_IN_INPUT,
		GATE_IN_INPUT,
		RESET_IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		ARP_OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	enum ArpModes
	{
		UP = 0,
		DOWN = 1,
		UP_DOWN = 2,
		DOWN_UP = 3
	};

	ArpModes mode = UP;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger gateTriggers[16];
	std::vector<int> gates;
	std::vector<float> voltages;

	dsp::Timer resetTimer;
	int step = 0;
	int channels = 0;
	bool toggle = false;
	bool wasDisconnected = true;

	Polyarp()
	{
		voltages.reserve(16);
		gates.reserve(16);
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TYPE_PARAM_PARAM, 0.f, 3.f, 0.f, "");
	}

	void updateChannels()
	{
		if (inputs[GATE_IN_INPUT].isConnected())
		{
			int newChannels = 0;
			std::vector<int> newGates(16);
			int gateChannels = inputs[GATE_IN_INPUT].getChannels();
			for (int i = 0; i < gateChannels; i++)
			{
				gateTriggers[i].process(rescale(inputs[GATE_IN_INPUT].getVoltage(i), 0.1f, 2.f, 0.f, 1.f));
				if (gateTriggers[i].isHigh())
				{
					newGates[newChannels] = i;
					newChannels += 1;
				}
			}
			if (newChannels != 0)
			{
				channels = newChannels;
				gates = newGates;
			}
		}
		else
		{
			channels = inputs[SIG_IN_INPUT].getChannels();
		}
	}

	void sampleHoldSort()
	{
		for (int i = 0; i < channels; i++)
		{
			voltages[i] = inputs[SIG_IN_INPUT].getVoltage(gates[i]);
		}
		std::sort(voltages.begin(), voltages.begin() + channels);
	}

	void nextStep()
	{
		if (channels == 1)
		{
			step = 0;
			return;
		}

		switch (mode)
		{
		case UP:
			step++;
			if (step >= channels)
			{
				step = 0;
			}
			break;
		case DOWN:
			step--;
			if (step < 0)
			{
				step = channels - 1;
			}
			break;
		case UP_DOWN:
			if (toggle)
			{
				step--;
				if (step < 0)
				{
					step += 2;
					toggle = !toggle;
				}
			}
			else
			{
				step++;
				if (step >= channels)
				{
					step -= 2;
					toggle = !toggle;
				}
			}
			break;

		case DOWN_UP:
			if (toggle)
			{
				step++;
				if (step >= channels)
				{
					step -= 2;
					toggle = !toggle;
				}
			}
			else
			{
				step--;
				if (step < 0)
				{
					step += 2;
					toggle = !toggle;
				}
			}
			break;

		default:
			break;
		}
	}
	void resetStep()
	{
		toggle = false;
		switch (mode)
		{
		case UP:
		case UP_DOWN:
			step = 0;
			break;

		case DOWN:
		case DOWN_UP:
			step = channels - 1;
			break;

		default:
			break;
		}
	}

	void process(const ProcessArgs &args) override
	{
		mode = (ArpModes)(int)rint(clamp(inputs[TYPE_IN_INPUT].getNormalVoltage(params[TYPE_PARAM_PARAM].getValue()), 0.0f, 3.0));
		bool reset = resetTrigger.process(rescale(inputs[RESET_IN_INPUT].getNormalVoltage(0.0f), 0.1f, 2.f, 0.f, 1.f));
		bool clock = clockTrigger.process(rescale(inputs[CLOCK_IN_INPUT].getNormalVoltage(0.0f), 0.1f, 2.f, 0.f, 1.f));
		updateChannels();
		if (channels == 0)
		{
			wasDisconnected = true;
			outputs[ARP_OUT_OUTPUT].setVoltage(0.0f);
		}
		else
		{
			if (wasDisconnected || reset)
			{
				resetStep();
				resetTimer.reset();
				sampleHoldSort();
			}
			if (clock && resetTimer.time > 1e-3f)
			{
				nextStep();
				sampleHoldSort();
			}
			outputs[ARP_OUT_OUTPUT].setVoltage(voltages[step]);
			wasDisconnected = false;
		}

		resetTimer.process(args.sampleTime);
	}
};

static const char *arpModeStrings[] = {
	"A",
	"B",
	"C",
	"D"};

struct PolyarpDisplay : TransparentWidget
{
	Polyarp *module;
	std::shared_ptr<Font> font;

	PolyarpDisplay()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Roboto/Roboto-Light.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		Polyarp::ArpModes mode = module ? module->mode : Polyarp::ArpModes::UP;

		// Background
		NVGcolor backgroundColor = nvgRGB(0x38, 0x38, 0x38);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontSize(args.vg, 24);
		nvgFontFaceId(args.vg, font->handle);
		// nvgTextLetterSpacing(args.vg, 1.5);

		Vec textPos = mm2px(Vec(10.583 / 2.0, 8.114 / 2.0));
		NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
		// nvgFillColor(args.vg, nvgTransRGBA(textColor, 16));
		// nvgText(args.vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, arpModeStrings[(int)mode], NULL);
	}
};

struct PolyarpWidget : ModuleWidget
{
	PolyarpWidget(Polyarp *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/frame.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// GEN_START
		auto ARP_OUT_POS = Vec(23, 337);
		auto TYPE_IN_POS = Vec(23, 86);
		auto SIG_IN_POS = Vec(23, 131);
		auto CLOCK_IN_POS = Vec(23, 176);
		auto RESET_IN_POS = Vec(23, 221);
		auto GATE_IN_POS = Vec(23, 266);
		auto TYPE_PARAM_POS = Vec(23, 32);
// GEN_END

		{
			Trimpot *knob = createParamCentered<Trimpot>(TYPE_PARAM_POS, module, Polyarp::TYPE_PARAM_PARAM);
			knob->snap = true;
			addParam(knob);
		}
		addOutput(createOutputCentered<PJ301MPort>(ARP_OUT_POS, module, Polyarp::ARP_OUT_OUTPUT));
		addInput(createInputCentered<PJ301MPort>(TYPE_IN_POS, module, Polyarp::TYPE_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(SIG_IN_POS, module, Polyarp::SIG_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(CLOCK_IN_POS, module, Polyarp::CLOCK_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(RESET_IN_POS, module, Polyarp::RESET_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(GATE_IN_POS, module, Polyarp::GATE_IN_INPUT));

		// mm2px(Vec(10.583, 7.475))
		// addChild(createWidget<Widget>(mm2px(Vec(2.272, 8.333))));

		// {
		// 	PolyarpDisplay *display = new PolyarpDisplay();
		// 	display->box.pos = mm2px(Vec(2.272, 8.333));
		// 	display->box.size = mm2px(Vec(10.583, 7.475));
		// 	// display->box.pos = display->box.pos.minus(display->box.size.div(2));
		// 	display->module = module;
		// 	addChild(display);
		// }
	}
};

Model *modelPolyarp = createModel<Polyarp, PolyarpWidget>("polyarp");