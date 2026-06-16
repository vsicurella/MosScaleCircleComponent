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
    ScaleStructureComponent& getEditor()  { return *editor; }

    void resized() override;

private:
    void updateModeButton();

    ScaleStructure scale;
    std::unique_ptr<ScaleStructureComponent> editor;

    juce::TextButton modeButton { "Colour mode" };
    juce::ToggleButton alwaysGroupNumsToggle  { "Always show group numbers" };
    juce::ToggleButton highlightToggle        { "Highlight on mouse over" };
    juce::ToggleButton highlightGroupNumToggle { "Highlight shows group number" };
    juce::ToggleButton showParamsToggle       { "Show parameters" };
    juce::ToggleButton showResizeToggle       { "Show group resize controls" };
    juce::ToggleButton showGroupsToggle       { "Show groups" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoComponent)
};
