/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.4.7

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2017 - ROLI Ltd.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --

#include "NumberSelector.h"
#include "GroupingCircle.h"
#include "ScaleStructure.h"
#include "TransparentDropDown.h"
#include "NoteNames.h"

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ScaleStructureComponent  : public Component,
                                 private NumberSelector::Listener,
                                 private GroupingCircle::Listener,
                                 private Button::Listener,
                                 private ScaleStructure::Listener
{
public:
    //==============================================================================
    // Primary constructor: colours resolved ByDegree. The component owns and manages internal
    // colour tables (seeded with a default palette) unless the host wires its own tables into
    // the ScaleStructure directly.
    ScaleStructureComponent (ScaleStructure& scaleStructureIn);

    // Backward-compatible constructor: colours resolved ByGroup using the caller-owned table.
    ScaleStructureComponent (ScaleStructure& scaleStructureIn, Array<Colour>& groupColourTableIn);

    ~ScaleStructureComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void paintOverChildren(Graphics& g) override;

	void buttonClicked(Button* buttonThatWasClicked) override;
	void selectorValueChanged(NumberSelector* selectorThatHasChanged) override;

	// GroupingCircle::Listener Implementation
	void offsetChanged(int newOffset) override;
	void degreeIndexAltered(int degree, Point<int> alteration) override;
	void allModificationsReset() override;
	void groupingSplit(int groupIndex, int sizeChangeAmount) override;
	void groupingResized(int groupIndex, int sizeChangeAmount, bool draggedClockwise) override;
	void groupingsMerged(int groupIndex) override;
	void groupColourChanged(int groupIndex, Colour newColour) override;
	void degreeColourChanged(int degreeIndex, Colour newColour) override;

	// ScaleStructure::Listener - keeps the UI in sync when the scale parameters change,
	// including changes pushed in by an external controller (follower mode).
	void scaleStructurePeriodChanged() override;
	void scaleStructureGeneratorChanged() override;
	void scaleStructureOffsetChanged() override;
	void scaleStructureSizeChanged() override;
	void scaleStructureGroupingChanged() override;
	void scaleStructureAlterationsChanged() override;

	void loadScaleStructureSettings();

	// Repaints the grouping circle after the colour scheme was edited externally.
	void refreshColours();

	void updateGenerators();
	void updateScaleSizes();
	void updatePeriodFactors();

	void setPeriod(int newPeriod);

	void onPeriodChange(bool sendNotification = true);
	void onPeriodFactorChange(int factorIndexIn);

	void onGeneratorChange(bool sendNotifcation = true);
	void onScaleSizeChange(bool sendNotification = true);

	void updatePGLabel();
	void updateLsLabel();
	void updateOffsetLabel();

	class Listener
	{
	public:
		virtual ~Listener() {}

		// Called when period changes
		virtual void scaleStructurePeriodChanged(int newPeriod) {};

		// Called whenever step sizes change
		virtual void scaleStructureStepSizesChanged(int rightUpwardSize, int horizontalSize) {};

		// Called when any property changes
		virtual void scaleStructureChanged() {};
	};

	void addListener(Listener* listenerIn);

	void removeListener(Listener* listenerIn);

protected:
	ListenerList<Listener> listeners;

public:
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

	// Functional Elements
	ScaleStructure& scaleStructure;
	GroupingCircle* circle;
	NoteNames noteNames;

	// Colour tables owned by this component when it manages colours itself (primary ctor).
	// When the host wires its own tables into the ScaleStructure, these stay unused.
	Array<Colour> ownedGroupColours;
	Array<Colour> ownedDegreeOverrides;
	bool ownsColourTables = false;

	// Shared construction body + colour-table maintenance.
	void buildComponent();
	void ensureColourTablesSized();
	static Colour defaultGroupColour(int groupIndex);

	// Components
	std::unique_ptr<NumberSelector> generatorSlider;
	std::unique_ptr<NumberSelector> periodSlider;
	std::unique_ptr<Label> generatorValueLbl;
	std::unique_ptr<Label> stepSizePatternLbl;
	std::unique_ptr<NumberSelector> sizeSelector;

	std::unique_ptr<Label> offsetLabel;
	Path offsetArrows;

	Path periodFactorButtonShape;
	std::unique_ptr<ShapeButton> periodFactorButton;
	PopupMenu periodFactorMenu;

	// Look and Feels
	std::unique_ptr<TransparentDropDown> generatorLookAndFeel;
	std::unique_ptr<TransparentDropDown> sizeLookAndFeel;
	std::unique_ptr<TransparentDropDown> periodFactorLookAndFeel;

	// Helpers
	int periodSelected;
	int generatorSelected;
	int generatorOffset = 0;

	float periodRatio = 2;
	float degreeCents = 100;
	float generatorCents = 7;
	float periodCents = 1200;

	Array<int> periodFactors;
	int periodFactorSelected = 0;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<GroupingCircle> circleComponent;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScaleStructureComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

