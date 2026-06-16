/*
  ==============================================================================

    DemoComponent.h
    Hosts a ScaleStructureComponent for manual verification of the module.

  ==============================================================================
*/

#pragma once

#include <mos_scale_circle/mos_scale_circle.h>

class DemoComponent : public juce::Component
{
public:
    DemoComponent();
    ~DemoComponent() override;

    void setColourMode (ScaleStructure::ColourMode mode);

    void resized() override;

private:
    void updateModeButton();

    ScaleStructure scale;
    std::unique_ptr<ScaleStructureComponent> editor;

    juce::TextButton modeButton { "Colour mode" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoComponent)
};
