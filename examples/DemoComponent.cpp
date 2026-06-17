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

    // Display toggles - initial states match the control's defaults.
    alwaysGroupNumsToggle.setToggleState  (false, juce::dontSendNotification);
    highlightToggle.setToggleState        (true,  juce::dontSendNotification);
    highlightGroupNumToggle.setToggleState(true,  juce::dontSendNotification);
    showParamsToggle.setToggleState       (true,  juce::dontSendNotification);
    showResizeToggle.setToggleState       (true,  juce::dontSendNotification);
    showGroupsToggle.setToggleState       (true,  juce::dontSendNotification);

    for (auto* t : { &alwaysGroupNumsToggle, &highlightToggle, &highlightGroupNumToggle,
                     &showParamsToggle, &showResizeToggle, &showGroupsToggle })
        addAndMakeVisible(t);

    alwaysGroupNumsToggle.onClick   = [this]() { editor->setAlwaysShowGroupNumbers  (alwaysGroupNumsToggle.getToggleState()); };
    highlightToggle.onClick         = [this]() { editor->setHighlightOnMouseOver     (highlightToggle.getToggleState()); };
    highlightGroupNumToggle.onClick = [this]() { editor->setHighlightShowsGroupNumber(highlightGroupNumToggle.getToggleState()); };
    showParamsToggle.onClick        = [this]() { editor->setShowParameters           (showParamsToggle.getToggleState()); };
    showResizeToggle.onClick        = [this]() { editor->setShowGroupResizeControls  (showResizeToggle.getToggleState()); };
    showGroupsToggle.onClick        = [this]() { editor->setShowGroups               (showGroupsToggle.getToggleState()); };

    // Scale option toggles — initial states match ScaleStructure defaults.
    attachModsToDegreeToggle.setToggleState(scale.isAlterationsAttachedToDegree(), juce::dontSendNotification);
    maintainSymmetryToggle.setToggleState  (scale.isRetainingSymmetry(),           juce::dontSendNotification);
    lockToMosSizesToggle.setToggleState    (scale.isRetainingMOSSizes(),           juce::dontSendNotification);

    for (auto* t : { &attachModsToDegreeToggle, &maintainSymmetryToggle, &lockToMosSizesToggle })
        addAndMakeVisible(t);

    attachModsToDegreeToggle.onClick = [this]() { scale.attachAlterationsToDegree    (attachModsToDegreeToggle.getToggleState()); editor->loadScaleStructureSettings(); };
    maintainSymmetryToggle.onClick   = [this]() { scale.setRetainGroupingSymmetry    (maintainSymmetryToggle.getToggleState()); };
    lockToMosSizesToggle.onClick     = [this]() { scale.setRetainMOSSizes            (lockToMosSizesToggle.getToggleState()); };

    setSize(900, 720);
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
    auto bounds = getLocalBounds().reduced(10);

    auto column = bounds.removeFromLeft(250);
    modeButton.setBounds(column.removeFromTop(30));
    column.removeFromTop(10);
    for (auto* t : { &alwaysGroupNumsToggle, &highlightToggle, &highlightGroupNumToggle,
                     &showParamsToggle, &showResizeToggle, &showGroupsToggle })
    {
        t->setBounds(column.removeFromTop(26));
        column.removeFromTop(4);
    }

    column.removeFromTop(10);
    for (auto* t : { &attachModsToDegreeToggle, &maintainSymmetryToggle, &lockToMosSizesToggle })
    {
        t->setBounds(column.removeFromTop(26));
        column.removeFromTop(4);
    }

    bounds.removeFromLeft(10);

    // Keep the wheel square so its radius/centre math stays correct.
    const int side = juce::jmin(bounds.getWidth(), bounds.getHeight());
    editor->setBounds(bounds.withSizeKeepingCentre(side, side));
}
