#include "plugin.hpp"

// LCM of 2,3,4,5,6,7,8,16,32
const int DIV_LCM = 3360;
// LCM of 2,3,4,5,6,7,8,12,16
const int MULT_LCM = 1680;

static const char *multDivStrings[] = {
	"32",
	"16",
	"8",
	"7",
	"6",
	"5",
	"4",
	"3",
	"2",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"12",
	"16"};

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

struct TappableClockSource : Module
{
	enum ParamIds
	{
		TAP_PARAM,
		MULT_DIV_PARAM,
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
		NUM_OUTPUTS
	};
	enum LightIds
	{
		TAP_LIGHT,
		NUM_LIGHTS
	};

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
	float timeSinceLastTap = 0.0f;
	bool tapping = false;
	bool hasEnoughTaps = false;
	int tap = 0;
	int multDiv = 0;
	int fullBeats = 0;
	int multBeats = 0;

	TappableClockSource()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TAP_PARAM, 0.f, 2.f, 0.f, "Tap");
		configParam(MULT_DIV_PARAM, -9.f, 9.f, 0.f, "Numerator");
		// configParam(DENOMINATOR_KNOB_PARAM, 0.f, 7.f, 0.f, "Denominator", "", 0.0f, 1.0f, 1.0f);
	}

	void process(const ProcessArgs &args) override
	{
		multDiv = (int)rint(clamp(inputs[MULT_DIV_INPUT].getNormalVoltage(params[MULT_DIV_PARAM].getValue()), -9.0, 9.0));
		bool clock = tapTrigger.process(rescale(inputs[TAP_INPUT].getNormalVoltage(params[TAP_PARAM].getValue()), 0.1f, 2.f, 0.f, 1.f));
		if (clock)
		{
			if (tapping)
			{
				float time = timer.process(args.sampleTime);
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
			float time = timer.process(args.sampleTime);
			if (time > maxTapDistance)
			{
				timer.reset();
				tapping = false;
				hasEnoughTaps = false;
			}
		}

		if (resetTrigger.process(rescale(inputs[RESET_INPUT].getNormalVoltage(0.0f), 0.1f, 2.f, 0.f, 1.f)))
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
		time += args.sampleTime;
		float multDuration = duration / (float)MULT_LCM;
		while (multTime >= multDuration)
		{
			multTime -= multDuration;
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
		multTime += args.sampleTime;

		lights[TAP_LIGHT].setSmoothBrightness(lightPulse.process(args.sampleTime) ? 1.0f : 0.0f, args.sampleTime);
		outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime) ? 10.0f : 0.0f);
		outputs[MULT_DIV_OUTPUT].setVoltage(multDivClockPulse.process(args.sampleTime) ? 10.0f : 0.0f);
	}
};

struct TappableClockSourceDisplay : TransparentWidget
{
	TappableClockSource *module;
	std::shared_ptr<Font> font;

	TappableClockSourceDisplay()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Roboto/Roboto-Light.ttf"));
	}

	void draw(const DrawArgs &args) override
	{
		int multDiv = module ? module->multDiv : 0;

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
		nvgText(args.vg, textPos.x, textPos.y, multDivStrings[multDiv + 9], NULL);
	}
};

struct TapLight : YellowLight
{
	TapLight()
	{
		box.size = Vec(28 - 6, 28 - 6);
		bgColor = color::BLACK_TRANSPARENT;
	}
};

struct KnobLight : YellowLight
{
	KnobLight()
	{
		box.size = Vec(7, 7);
		bgColor = color::BLACK_TRANSPARENT;
	}
};

struct TappableClockSourceWidget : ModuleWidget
{
	TappableClockSourceWidget(TappableClockSource *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TappableClockSource.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// GEN_START
		Vec CLOCK_IN_POS = Vec(22.5, 86);
		Vec CLOCK_OUT_POS = Vec(22.5, 131);
		Vec MULT_DIV_IN_POS = Vec(22.5, 221);
		Vec MULT_DIV_OUT_POS = Vec(22.5, 337);
		Vec MULT_DIV_PARAM_POS = Vec(22.5, 177);
		Vec RESET_IN_POS = Vec(22.5, 266);
		Vec TAP_PARAM_POS = Vec(22.5, 32);
// GEN_END

		addParam(createParamCentered<CKD6>(TAP_PARAM_POS, module, TappableClockSource::TAP_PARAM));
		addChild(createLightCentered<TapLight>(TAP_PARAM_POS, module, TappableClockSource::TAP_LIGHT));
		addInput(createInputCentered<PJ301MPort>(CLOCK_IN_POS, module, TappableClockSource::TAP_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(CLOCK_OUT_POS, module, TappableClockSource::CLOCK_OUTPUT));

		// {
		// 	TappableClockSourceDisplay *display = new TappableClockSourceDisplay();
		// 	display->box.pos = mm2px(Vec(7.62, 56.521));
		// 	display->box.size = mm2px(Vec(10.583, 7.476));
		// 	display->box.pos = display->box.pos.minus(display->box.size.div(2));
		// 	display->module = module;
		// 	addChild(display);
		// }

		{
			auto knob = createParamCentered<RoundSmallBlackKnob>(MULT_DIV_PARAM_POS, module, TappableClockSource::MULT_DIV_PARAM);
			knob->snap = true;
			addParam(knob);
		}
		addInput(createInputCentered<PJ301MPort>(MULT_DIV_IN_POS, module, TappableClockSource::MULT_DIV_INPUT));
		addInput(createInputCentered<PJ301MPort>(RESET_IN_POS, module, TappableClockSource::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(MULT_DIV_OUT_POS, module, TappableClockSource::MULT_DIV_OUTPUT));
	}
};

Model *modelTappableClockSource = createModel<TappableClockSource, TappableClockSourceWidget>("TappableClockSource");