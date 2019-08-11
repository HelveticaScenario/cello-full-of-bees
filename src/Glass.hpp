#include <rack.hpp>
using namespace rack;

struct KnobLightsWidget : YellowLight
{
    // Glass *module = NULL;
    float minAngle = -0.83 * M_PI;
    float maxAngle = 0.83 * M_PI;
    int numSections = 25;
    float knobValue = 0.0f;
    float cvValue = 0.0f;

    KnobLightsWidget();


    void draw(const DrawArgs &args) override;
};