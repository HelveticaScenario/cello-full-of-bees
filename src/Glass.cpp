#include "plugin.hpp"
// #include "Glass.hpp"

struct Glass : Module
{
	// KnobLightsWidget *knobLight = NULL;
	enum ParamIds
	{
		OFFSET_KNOB_PARAM,
		OFFSET_ATTENUVERT_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		// ENUMS(OFFSET_LIGHT, 25),
		// OFFSET_LIGHT,
		NUM_LIGHTS
	};

	Glass()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override
	{
		// lights[Glass::OFFSET_LIGHT].setBrightness(1.0f);
		float knobVal = params[Glass::OFFSET_KNOB_PARAM].getValue();
		// knobLight->knobValue = knobVal;
		// knobLight->numSections = 13;
		// if (inputs[Glass::IN_INPUT].isConnected())
		// {
		// 	knobLight->cvValue = math::rescale(inputs[Glass::IN_INPUT].getVoltage(), -12.0f, 12.0f, 0.0f, 1.0f);
		// }
		// else
		// {
		// 	knobLight->cvValue = knobVal;
		// }

		// knobLight->
	}
};

// KnobLightsWidget::KnobLightsWidget()
// {
// 	box.size = Vec(55, 55);

// 	// bgColor = color::BLACK_TRANSPARENT;
// };

// void KnobLightsWidget::draw(const DrawArgs &args)
// {
// 	// 	// module
// 	// 	// float amplitude = module ? module->amplitude : 1.f;

// 	// 	// nvgBeginPath(args.vg);
// 	// 	// nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
// 	// 	// nvgFillColor(args.vg, nvgRGB(0, 0, 0));
// 	// 	// nvgFill(args.vg);

// 	// 	// const int segs = 25;
// 	// 	// const Vec margin = Vec(3, 3);
// 	// 	// Rect r = box.zeroPos().grow(margin.neg());

// 	// 	// for (int i = 0; i < segs; i++)
// 	// 	// {
// 	// 	// 	float value = paramQuantity ? paramQuantity->getValue() : 1.f;
// 	// 	// 	float segValue = clamp(value * segs - (segs - i - 1), 0.f, 1.f);
// 	// 	// 	float segAmplitude = clamp(amplitude * segs - (segs - i - 1), 0.f, 1.f);
// 	// 	// 	nvgBeginPath(args.vg);
// 	// 	// 	nvgRect(args.vg, r.pos.x, r.pos.y + r.size.y / segs * i + 0.5,
// 	// 	// 			r.size.x, r.size.y / segs - 1.0);
// 	// 	// 	if (segValue > 0.f)
// 	// 	// 	{
// 	// 	// 		nvgFillColor(args.vg, color::alpha(nvgRGBf(0.33, 0.33, 0.33), segValue));
// 	// 	// 		nvgFill(args.vg);
// 	// 	// 	}
// 	// 	// 	if (segAmplitude > 0.f)
// 	// 	// 	{
// 	// 	// 		nvgFillColor(args.vg, color::alpha(SCHEME_GREEN, segAmplitude));
// 	// 	// 		nvgFill(args.vg);
// 	// 	// 	}
// 	// 	// }

// 	// 	// std::cout << "hello" << std::endl;
// 	float radius = box.size.x / 2.0;
// 	float arcStart = minAngle - M_PI_2;
// 	float arcEnd = maxAngle - M_PI_2;
// 	float difference = abs(arcStart - arcEnd);
// 	int sections = numSections;
// 	float spacer = 0.04;
// 	float sectionWidth = difference / (float)(sections);
// 	float start = spacer / 2.0f;
// 	float sectionRange = 1.0f / (float)(sections);

// 	for (size_t i = 0; i < sections; i++)
// 	{

// 		float sectionStart = arcStart + (sectionWidth * (float)i) + (spacer / 2.0f);
// 		float sectionEnd = arcStart + (sectionWidth * (float)(i + 1)) - (spacer / 2.0f);

// 		nvgBeginPath(args.vg);
// 		nvgArc(args.vg, radius, radius, radius, sectionStart, sectionEnd, NVG_CW);
// 		nvgArc(args.vg, radius, radius, 45.0f / 2.0f, sectionEnd, sectionStart, NVG_CCW);
// 		nvgClosePath(args.vg);
// 		float rangeStart = (sectionRange * i);
// 		float rangeEnd = math::clamp(rangeStart + sectionRange, 0.0f, 1.0f);
// 		float midPoint = (sectionRange * i) + (sectionRange / 2.0f);

// 		// Background
// 		if (bgColor.a > 0.0)
// 		{
// 			nvgFillColor(args.vg, bgColor);
// 			nvgFill(args.vg);
// 		}

// 		// Foreground
// 		if (knobValue >= rangeStart && knobValue <= rangeEnd)
// 		{
// 			nvgFillColor(args.vg, nvgRGBAf(color.r, color.g, color.b, 1.0f));
// 			nvgFill(args.vg);
// 		}
// 		else if (knobValue < rangeStart && knobValue >= (midPoint - sectionRange))
// 		{
// 			nvgFillColor(args.vg, nvgRGBAf(color.r, color.g, color.b, pow(math::rescale(knobValue, midPoint - sectionRange, rangeStart, 0.0f, 1.0f), 2.0f)));
// 			nvgFill(args.vg);
// 		}
// 		else if (knobValue > rangeEnd && knobValue <= (midPoint + sectionRange))
// 		{
// 			nvgFillColor(args.vg, nvgRGBAf(color.r, color.g, color.b, pow(math::rescale(knobValue, rangeEnd, midPoint + sectionRange, 0.0f, 1.0f), 2.0f)));
// 			nvgFill(args.vg);
// 		}

// 		// Border
// 		if (borderColor.a > 0.0)
// 		{
// 			nvgStrokeWidth(args.vg, 0.5);
// 			nvgStrokeColor(args.vg, borderColor);
// 			nvgStroke(args.vg);
// 		}
// 		// start += sectionWidth + spacer;
// 		// rangeStart += sectionRange;
// 		/* code */
// 	}
// };

struct GlassWidget : ModuleWidget
{
	GlassWidget(Glass *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Glass.svg")));
		float widthUnit = box.size.x * RACK_GRID_WIDTH / 16.0f;
		float heightUnit = box.size.y * RACK_GRID_HEIGHT /64.0f;
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		Vec offsetCenter = Vec(RACK_GRID_WIDTH * 8.0f / 2.0f, 60.0f);
		addParam(createParamCentered<RoundLargeBlackKnob>(offsetCenter, module, Glass::OFFSET_KNOB_PARAM));
		// KnobLightsWidget *knobLightsWidget = createLightCentered<KnobLightsWidget>(offsetCenter, module, Glass::OFFSET_LIGHT);
		// module->knobLight = knobLightsWidget;
		// addChild(knobLightsWidget);
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.51398, 97.74977)), module, Glass::IN_INPUT));

		// for (size_t i = 0; i < 25; i++)
		// {
		// 	/* code */
		// 	float inc = (float)i / (float)(25 - 1);
		// 	float circ = M_PI * 2.0f;
		// 	float slice = 1.2f;
		// 	float arc = circ - slice;
		// 	float s = std::sin((inc * arc) + (slice / 2.0f));
		// 	float c = std::cos((inc * arc) + (slice / 2.0f));
		// 	addChild(createLightCentered<KnobLightsWidget>((Vec(RACK_GRID_WIDTH * 8.0f / 2.0f + (s * 30.0f), 60.0f + (c * 30.0f))), module, Glass::OFFSET_LIGHT + i));
		// }
	}
};

Model *modelGlass = createModel<Glass, GlassWidget>("glass");