/*
  ==============================================================================

    DemoComponent.cpp

  ==============================================================================
*/

#include "DemoComponent.h"

DemoComponent::DemoComponent()
{
    // A simple 12-note period; suggested generator (-1) and size (-1).
    scale.setAll(12, -1, -1);

    // Primary constructor: ByDegree colours, component-owned tables seeded with a palette.
    editor.reset(new ScaleStructureComponent(scale));
    addAndMakeVisible(*editor);

    // Demonstrate per-degree overrides: these degrees differ from their group colour in
    // ByDegree mode, and are ignored (follow the group) in ByGroup mode.
    scale.setDegreeColour(0, juce::Colours::red);
    scale.setDegreeColour(4, juce::Colours::white);
    scale.setDegreeColour(9, juce::Colours::black);

    addAndMakeVisible(modeButton);
    modeButton.onClick = [this]()
    {
        auto next = (scale.getColourMode() == ScaleStructure::ColourMode::ByDegree)
                        ? ScaleStructure::ColourMode::ByGroup
                        : ScaleStructure::ColourMode::ByDegree;
        scale.setColourMode(next);
        editor->refreshColours();
        updateModeButton();
    };
    updateModeButton();

    setSize(760, 800);
}

DemoComponent::~DemoComponent() = default;

void DemoComponent::setColourMode (ScaleStructure::ColourMode mode)
{
    scale.setColourMode(mode);
    editor->refreshColours();
    updateModeButton();
}

void DemoComponent::updateModeButton()
{
    modeButton.setButtonText(scale.getColourMode() == ScaleStructure::ColourMode::ByDegree
                                 ? "Colour mode: ByDegree"
                                 : "Colour mode: ByGroup");
}

void DemoComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    modeButton.setBounds(bounds.removeFromTop(28).removeFromLeft(220));
    bounds.removeFromTop(8);
    editor->setBounds(bounds);
}
