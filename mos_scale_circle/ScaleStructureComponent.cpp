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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "ScaleStructureComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ScaleStructureComponent::ScaleStructureComponent (ScaleStructure& scaleStructureIn)
    : scaleStructure(scaleStructureIn), noteNames(scaleStructureIn)
{
    // This component owns its colour tables and resolves colours ByDegree by default.
    ownsColourTables = true;
    for (int i = 0; i < 12; ++i)
        ownedGroupColours.add(defaultGroupColour(i));

    scaleStructure.setColourMode(ScaleStructure::ColourMode::ByDegree);
    scaleStructure.setGroupColourTable(ownedGroupColours);
    scaleStructure.setDegreeOverrideTable(ownedDegreeOverrides);

    buildComponent();
}

ScaleStructureComponent::ScaleStructureComponent (ScaleStructure& scaleStructureIn, Array<Colour>& groupColourTableIn)
    : scaleStructure(scaleStructureIn), noteNames(scaleStructureIn)
{
    // Backward-compatible path: colours come from the caller-owned table, resolved ByGroup.
    scaleStructure.setColourMode(ScaleStructure::ColourMode::ByGroup);
    scaleStructure.setGroupColourTable(groupColourTableIn);

    buildComponent();
}

void ScaleStructureComponent::buildComponent()
{
    ensureColourTablesSized();

    circleComponent.reset (new GroupingCircle (scaleStructure));
    addAndMakeVisible (circleComponent.get());
    circleComponent->setName ("circleComponent");


    //[UserPreSize]
	// Set up components
	periodSlider.reset(new NumberSelector("Period"));
	addAndMakeVisible(periodSlider.get());
	periodSlider->showNameLabel();
	periodSlider->setRange(5, 275, true, false);
	periodSlider->addListener(this);

	generatorSlider.reset(new NumberSelector("Generator", NumberSelector::SelectionType::List));
	addAndMakeVisible(generatorSlider.get());
	generatorSlider->showNameLabel();
	generatorSlider->addListener(this);

	generatorLookAndFeel.reset(new TransparentDropDown());
	// TODO: add SSC colour ids
	//generatorLookAndFeel.setBaseColour(findColour(ColourIds::backgroundColourId));
	generatorLookAndFeel->setColour(PopupMenu::ColourIds::backgroundColourId, Colour());
	generatorSlider->setLookAndFeel(generatorLookAndFeel.get());

	generatorValueLbl.reset(new Label("generatorValueLbl"));
	addAndMakeVisible(generatorValueLbl.get());
	generatorValueLbl->setFont(Font(16.0f, Font::plain).withTypefaceStyle("Regular"));
	generatorValueLbl->setJustificationType(Justification::centred);
	generatorValueLbl->setEditable(false, false, false);
	generatorValueLbl->setColour(TextEditor::textColourId, Colours::black);
	generatorValueLbl->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
	generatorValueLbl->setInterceptsMouseClicks(false, false);

	Colour seedColour = scaleStructure.getGroupColour(0);
	if (seedColour.isTransparent())
		seedColour = Colours::grey;

	sizeSelector.reset(new NumberSelector("Scale Size", NumberSelector::SelectionType::List, seedColour.contrasting(0.8f)));
	addAndMakeVisible(sizeSelector.get());
	sizeSelector->showNameLabel();
	sizeSelector->addListener(this);

	sizeLookAndFeel.reset(new TransparentDropDown());
	sizeLookAndFeel->setBaseColour(seedColour);
	sizeLookAndFeel->setColour(PopupMenu::ColourIds::backgroundColourId, Colour());
	sizeLookAndFeel->setColour(ComboBox::ColourIds::textColourId, sizeSelector->findColour(NumberSelector::ColourIds::valueTextColourId));
	sizeSelector->setLookAndFeel(sizeLookAndFeel.get());

	stepSizePatternLbl.reset(new Label("stepSizePatternLbl"));
	addAndMakeVisible(stepSizePatternLbl.get());
	stepSizePatternLbl->setFont(Font(16.0f, Font::plain).withTypefaceStyle("Regular"));
	stepSizePatternLbl->setJustificationType(Justification::centred);
	stepSizePatternLbl->setEditable(false, false, false);
	stepSizePatternLbl->setColour(TextEditor::textColourId, Colours::black);
	stepSizePatternLbl->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
	stepSizePatternLbl->setInterceptsMouseClicks(false, false);

	offsetLabel.reset(new Label("offsetLabel"));
	offsetLabel->setJustificationType(Justification::centred);
	offsetLabel->setColour(Label::ColourIds::textColourId, Colours::white);
	addAndMakeVisible(offsetLabel.get());

	// TODO: add SSC component colour ids
	//periodFactorLookAndFeel.setBaseColour(findColour(ColourIds::backgroundColourId));
	periodFactorButton.reset(new ShapeButton("periodFactorButton", Colours::white, Colours::white.contrasting(0.125f), Colours::white.contrasting(0.25f)));
	periodFactorButton->setTooltip("Number of periods.");
	addChildComponent(periodFactorButton.get());
	periodFactorButton->addListener(this);

	periodFactorLookAndFeel.reset(new TransparentDropDown());
	periodFactorMenu.setLookAndFeel(periodFactorLookAndFeel.get());

	circle = dynamic_cast<GroupingCircle*>(circleComponent.get());
	circle->addListener(this);

	circle->setNoteNameSystem(&noteNames);

	loadScaleStructureSettings();

	// Observe the model so external (follower-mode) parameter changes refresh the UI.
	scaleStructure.addListener(this);
    //[/UserPreSize]

    setSize (600, 600);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ScaleStructureComponent::~ScaleStructureComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
	scaleStructure.removeListener(this);
	listeners.clear();
    //[/Destructor_pre]

    circleComponent = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
	generatorSlider = nullptr;
	periodSlider = nullptr;
	generatorValueLbl = nullptr;
	stepSizePatternLbl = nullptr;
	sizeSelector = nullptr;
	periodFactorMenu.setLookAndFeel(nullptr);
    //[/Destructor]
}

//==============================================================================
void ScaleStructureComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..

	// Offset Label arrows (part of the parameter UI)
	if (showParameters)
	{
		g.setColour(Colours::white);
		PathStrokeType strokeType(1.0f);
		g.strokePath(offsetArrows, strokeType);
	}
    //[/UserPaint]
}

void ScaleStructureComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    circleComponent->setBounds (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));
    //[UserResized] Add your own custom resize handling here..

	// TODO: implement (probably ex-projucer) this so that the bounds don't have to be set twice
	periodSlider->setSize(proportionOfWidth(0.2f), proportionOfHeight(0.155f));
	periodSlider->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 0.4f, 0));

	generatorSlider->setSize(proportionOfWidth(0.2f), proportionOfHeight(0.155f));
	generatorSlider->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 0.125f, MathConstants<float>::pi));

	generatorValueLbl->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 2.0f / 3.0f, MathConstants<float>::pi * 11.0f / 8.0f));
	stepSizePatternLbl->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 2.0f / 3.0f, MathConstants<float>::pi * 5.0f / 8.0f));

	offsetLabel->setFont(Font().withHeight(getHeight() / 48.0f));
	offsetLabel->setSize(offsetLabel->getFont().getStringWidth("Offset") * 2, offsetLabel->getFont().getHeight() * 3);
	offsetLabel->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 7.0f / 8.0f, 0));

	// Default (group ring shown): the Scale Size selector sits in the group ring at the top.
	// The groups-hidden override below relocates it into the hub.
	sizeSelector->setSize(proportionOfWidth(0.175f), (circle->getOuterRadius() - circle->getMiddleRadius()) * 7.0f / 8.0f);
	sizeSelector->setCentrePosition(circle->getIntPointFromCenter((circle->getOuterRadius() + circle->getMiddleRadius()) / 2.025f, 0));

	// TODO: add other part of arrow / improve drawing
	offsetArrows.clear();
	Rectangle<float> offsetLabelCircleBounds = circle->getInnerCircleBounds().reduced(circle->getInnerRadius() / 13.0f);
	GroupingCircle::addArcToPath(offsetArrows, offsetLabelCircleBounds, MathConstants<float>::pi / 24, MathConstants<float>::pi / 12, true);
	offsetArrows.lineTo(circle->getFloatPointFromCenter(circle->getInnerRadius() * 13.0f / 14.0f, MathConstants<float>::pi / 14));
	GroupingCircle::addArcToPath(offsetArrows, offsetLabelCircleBounds, -MathConstants<float>::pi / 24, -MathConstants<float>::pi / 12, true);
	offsetArrows.lineTo(circle->getFloatPointFromCenter(circle->getInnerRadius() * 13.0f / 14.0f, -MathConstants<float>::pi / 14));

	float periodFBtnSize = periodSlider->getHeight() / 8.0f;
	periodFactorButtonShape.clear();
	periodFactorButtonShape.addEllipse(periodSlider->getRight(), periodSlider->getY(), periodFBtnSize, periodFBtnSize);
	periodFactorButton->setShape(periodFactorButtonShape, true, true, true);
	periodFactorButton->setTopLeftPosition(periodSlider->getPosition().translated(periodSlider->getWidth() * 4 / 5.0f, 0));

	generatorValueLbl->setSize(getWidth(), proportionOfHeight(0.15f));
	generatorValueLbl->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 3.0f / 7.0f, MathConstants<float>::pi));

	stepSizePatternLbl->setSize(getWidth(), proportionOfHeight(0.15f));
	stepSizePatternLbl->setCentrePosition(circle->getIntPointFromCenter(circle->getInnerRadius() * 4.0 / 7.0f, MathConstants<float>::pi));

	// Group ring hidden: rebalance the hub into a symmetric stack and slot the Scale Size
	// selector between the Offset label (top) and the Period control.
	if (!showGroups)
	{
		const float inner = circle->getInnerRadius();
		const float pi = MathConstants<float>::pi;

		// Preserve the groups-shown spacing of Period / Generator / cents / steps; just shift the
		// whole cluster straight down by a fixed amount to free room above Period for Scale Size.
		const int shift = roundToInt(inner * 0.18f);

		periodSlider->setCentrePosition(circle->getIntPointFromCenter(inner * 0.4f, 0).translated(0, shift));
		generatorSlider->setCentrePosition(circle->getIntPointFromCenter(inner * 0.125f, pi).translated(0, shift));
		generatorValueLbl->setCentrePosition(circle->getIntPointFromCenter(inner * 3.0f / 7.0f, pi).translated(0, shift));
		stepSizePatternLbl->setCentrePosition(circle->getIntPointFromCenter(inner * 4.0f / 7.0f, pi).translated(0, shift));

		// Scale Size sits in the freed space between the Offset label and the (shifted) Period control.
		sizeSelector->setSize(proportionOfWidth(0.2f), proportionOfHeight(0.12f));
		sizeSelector->setCentrePosition(circle->getIntPointFromCenter(inner * 0.55f, 0));

		// Re-place the period-factor button next to the moved Period control.
		const float periodFBtnSize = periodSlider->getHeight() / 8.0f;
		periodFactorButtonShape.clear();
		periodFactorButtonShape.addEllipse(periodSlider->getRight(), periodSlider->getY(), periodFBtnSize, periodFBtnSize);
		periodFactorButton->setShape(periodFactorButtonShape, true, true, true);
		periodFactorButton->setTopLeftPosition(periodSlider->getPosition().translated(periodSlider->getWidth() * 4 / 5.0f, 0));
	}

    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ScaleStructureComponent::paintOverChildren(Graphics& g)
{
	// Period Factor label
	if (periodFactorButton->isVisible())
	{
		g.setColour(Colours::black);
		g.setFont(periodFactorButton->getHeight() * 7 / 8.0f );
		g.drawFittedText(String(scaleStructure.getPeriodFactor()), periodFactorButton->getBounds().translated(0, -1), Justification::centred, 1);
	}
}

void ScaleStructureComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == periodFactorButton.get())
	{
		periodFactorMenu.clear();
		for (int i = 0; i < periodFactors.size(); i++)
		{
			periodFactorMenu.addItem(i + 1, String(periodFactors[i]), true, i == scaleStructure.getPeriodFactorIndex());
		}

		PopupMenu::Options options = PopupMenu::Options()
			.withMinimumWidth(periodSlider->getHeight() / 4)
			.withStandardItemHeight(periodSlider->getHeight() / 4)
			.withTargetComponent(periodFactorButton.get());

		periodFactorMenu.showMenuAsync(options, [=](int choice)
		{
			if (choice > 0)
				onPeriodFactorChange(choice - 1);
		});
	}
}

void ScaleStructureComponent::selectorValueChanged(NumberSelector* selectorThatHasChanged)
{
	if (selectorThatHasChanged == periodSlider.get())
	{
		onPeriodChange();
	}

	else if (selectorThatHasChanged == generatorSlider.get())
	{
		generatorSelected = generatorSlider->getIndex();
		DBG("\nSSC: Generator index set to: " + String(generatorSelected));
		scaleStructure.setAll(periodSelected, generatorSelected, -1, generatorOffset, periodFactorSelected);

		generatorOffset = scaleStructure.getGeneratorOffset();
		generatorCents = degreeCents * scaleStructure.getGenerator();

		updatePGLabel();
		updateScaleSizes();
	}

	else if (selectorThatHasChanged == sizeSelector.get())
	{
		DBG("\nSSC: Size index set to: " + String(sizeSelector->getIndex() + 1));
		scaleStructure.setSizeIndex(sizeSelector->getIndex() + 1);
	}

	listeners.call(&ScaleStructureComponent::Listener::scaleStructureStepSizesChanged,
		scaleStructure.getStepSize().x,
		scaleStructure.getStepSize().y);

	noteNames.useAlphabeticalDefault();

	generatorOffset = scaleStructure.getGeneratorOffset();
	circle->updateGenerator();

	updateLsLabel();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::offsetChanged(int newOffset)
{
	DBG("\nSSC: Generator Offset set to: " + String(newOffset));
	generatorOffset = newOffset;
	scaleStructure.setGeneratorOffset(generatorOffset);

	updateOffsetLabel();
	updateLsLabel();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::degreeIndexAltered(int degreeIndex, Point<int> alteration)
{
	DBG("\nSSC: Degree Index swapped: " + String(degreeIndex) + " (" + alteration.toString() + ")");

	if (alteration.x >= 0 && alteration.y != 0)
		scaleStructure.setAlterationOfDegree(degreeIndex, alteration);
	else
		scaleStructure.resetAlterationOfDegree(degreeIndex);

	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
	updateLsLabel();
}

void ScaleStructureComponent::allModificationsReset()
{
	scaleStructure.setChromaAlterations(Array<Point<int>>());
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
	updateLsLabel();
}

void ScaleStructureComponent::groupingSplit(int groupIndex, int sizeChangeAmount)
{
	DBG("SSC: Group " + String(groupIndex) + " split with new size " + String(sizeChangeAmount));
	scaleStructure.splitDegreeGroup(groupIndex, sizeChangeAmount);
	ensureColourTablesSized();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::groupingResized(int groupIndex, int sizeChangeAmount, bool draggedClockwise)
{
	DBG("SSC: Group " + String(groupIndex) + " resized by " + String(sizeChangeAmount) +", " + (!draggedClockwise ? "counter" : "") + "clockwise");
	scaleStructure.resizeDegreeGroup(groupIndex, sizeChangeAmount, draggedClockwise);
	ensureColourTablesSized();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::groupingsMerged(int groupIndex)
{
	DBG("SSC: Group " + String(groupIndex) + " merged with group " + String(groupIndex - 1));
	scaleStructure.mergeDegreeGroups(groupIndex);
	ensureColourTablesSized();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::groupColourChanged(int groupIndex, Colour newColour)
{
	scaleStructure.setGroupColour(groupIndex, newColour);
	refreshColours();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::degreeColourChanged(int degreeIndex, Colour newColour)
{
	scaleStructure.setDegreeColour(degreeIndex, newColour);
	refreshColours();
	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::refreshColours()
{
	if (circle != nullptr)
		circle->repaint();
}

//==============================================================================
// Display toggles

void ScaleStructureComponent::setShowParameters(bool shouldShow)
{
	showParameters = shouldShow;

	periodSlider->setVisible(shouldShow);
	generatorSlider->setVisible(shouldShow);
	sizeSelector->setVisible(shouldShow);
	generatorValueLbl->setVisible(shouldShow);
	stepSizePatternLbl->setVisible(shouldShow);
	offsetLabel->setVisible(shouldShow);

	if (shouldShow)
		updatePeriodFactors();          // restores the period-factor button per factor count
	else
		periodFactorButton->setVisible(false);

	repaint();                          // the offset arrows are painted only when shown
}

void ScaleStructureComponent::setAlwaysShowGroupNumbers(bool shouldShow)
{
	if (circle != nullptr)
		circle->setAlwaysShowGroupNumbers(shouldShow);
}

void ScaleStructureComponent::setHighlightOnMouseOver(bool shouldHighlight)
{
	if (circle != nullptr)
		circle->setHighlightOnMouseOver(shouldHighlight);
}

void ScaleStructureComponent::setHighlightShowsGroupNumber(bool shouldShow)
{
	if (circle != nullptr)
		circle->setHighlightShowsGroupNumber(shouldShow);
}

void ScaleStructureComponent::setShowGroupResizeControls(bool shouldShow)
{
	if (circle != nullptr)
		circle->setShowGroupResizeControls(shouldShow);
}

void ScaleStructureComponent::setShowGroups(bool shouldShow)
{
	showGroups = shouldShow;
	if (circle != nullptr)
		circle->setShowGroups(shouldShow);
	applySizeSelectorColours();   // dark text on the ring, white text in the hub
	resized();                    // the Scale Size selector moves into the hub when groups are hidden
}

void ScaleStructureComponent::applySizeSelectorColours()
{
	if (showGroups)
	{
		Colour seed = scaleStructure.getGroupColour(0);
		if (seed.isTransparent())
			seed = Colours::grey;

		sizeSelector->setTextColour(seed.contrasting(0.8f));
		sizeLookAndFeel->setBaseColour(seed);
	}
	else
	{
		Colour hubBase = Colours::darkgrey;
		sizeSelector->setTextColour(Colours::white);
		sizeLookAndFeel->setBaseColour(hubBase);
	}
}

//==============================================================================
// ScaleStructure::Listener - refresh the UI from the model (follower mode). These read the
// model and set widgets without notifying, so they never write back to the model.

void ScaleStructureComponent::scaleStructurePeriodChanged()
{
	loadScaleStructureSettings();
}

void ScaleStructureComponent::scaleStructureGeneratorChanged()
{
	generatorOffset = scaleStructure.getGeneratorOffset();
	updateGenerators();
	updateScaleSizes();
	updatePGLabel();
	updateLsLabel();
	if (circle != nullptr)
		circle->updateGenerator();
}

void ScaleStructureComponent::scaleStructureOffsetChanged()
{
	generatorOffset = scaleStructure.getGeneratorOffset();
	updateOffsetLabel();
	updateLsLabel();
	if (circle != nullptr)
		circle->updateGenerator();
}

void ScaleStructureComponent::scaleStructureSizeChanged()
{
	updateScaleSizes();
	updateLsLabel();
	if (circle != nullptr)
		circle->updateGenerator();
}

void ScaleStructureComponent::scaleStructureGroupingChanged()
{
	ensureColourTablesSized();
	if (circle != nullptr)
		circle->updateGenerator();
}

void ScaleStructureComponent::scaleStructureAlterationsChanged()
{
	updateLsLabel();
	if (circle != nullptr)
		circle->updateGenerator();
}

Colour ScaleStructureComponent::defaultGroupColour(int groupIndex)
{
	// Distinct hues spaced by the golden ratio for good separation at any count.
	return Colour::fromHSV(std::fmod(0.075f + groupIndex * 0.6180339887f, 1.0f), 0.55f, 0.92f, 1.0f);
}

void ScaleStructureComponent::ensureColourTablesSized()
{
	if (!ownsColourTables || !scaleStructure.isValid())
		return;

	// One colour per group; seed any newly-added group slots from the default palette.
	const int numGroups = scaleStructure.getDegreeGroupings().size();
	for (int i = ownedGroupColours.size(); i < numGroups; ++i)
		ownedGroupColours.add(defaultGroupColour(i));

	// One override slot per scale degree; new entries default to transparent ("no override").
	const int period = scaleStructure.getPeriod();
	if (ownedDegreeOverrides.size() != period)
		ownedDegreeOverrides.resize(period);
}

void ScaleStructureComponent::loadScaleStructureSettings()
{
	if (scaleStructure.isValid())
	{
		periodSelected = scaleStructure.getPeriod();

		periodSlider->setValue(periodSelected, false);
		periodCents = log2f(periodRatio / scaleStructure.getPeriodFactor()) * 1200;
		degreeCents = periodCents / periodSelected;

		updatePeriodFactors();
		updateGenerators();
		updatePGLabel();
		updateScaleSizes();
		updateLsLabel();
		updateOffsetLabel();

		ensureColourTablesSized();

		circle->updatePeriod();
		circle->updateGenerator();
	}
}

void ScaleStructureComponent::updateGenerators()
{
	generatorSlider->setList(scaleStructure.getValidGenerators(), false, false);

	generatorSelected = scaleStructure.getGeneratorIndex();
	generatorSlider->setIndex(generatorSelected, false);

	generatorCents = degreeCents * scaleStructure.getGenerator();
}

void ScaleStructureComponent::updateScaleSizes()
{
	Array<int> sizes = scaleStructure.getScaleSizes();
	sizes.remove(0); // First size is redundant size of 1
	if (scaleStructure.getPeriodFactor() > 1)
	{
		for (int i = 0; i < sizes.size(); i++)
		{
			sizes.set(i, sizes[i] * scaleStructure.getPeriodFactor());
		}
	}

	sizeSelector->setList(sizes, false, false);
	sizeSelector->setIndex(jlimit(0, sizes.size(), scaleStructure.getScaleSizeIndex() - 1), false);
}

void ScaleStructureComponent::updatePeriodFactors()
{
	periodFactors = scaleStructure.getPeriodFactors();
	periodFactorSelected = scaleStructure.getPeriodFactorIndex();

	if (periodFactors.size() == 1)
		periodFactorButton->setVisible(false);
	else
		periodFactorButton->setVisible(true);
}

void ScaleStructureComponent::setPeriod(int newPeriod)
{
	periodSlider->setValue(newPeriod);
}

void ScaleStructureComponent::onPeriodChange(bool sendNotification)
{
	periodSelected = periodSlider->getValue();
	DBG("\nSSC: Period set to: " + String(periodSelected));
	scaleStructure.setAll(periodSelected, -1, -1, generatorOffset, 0);
	updatePeriodFactors();

	periodCents = log2f(periodRatio / scaleStructure.getPeriodFactor()) * 1200;
	degreeCents = periodCents / scaleStructure.getPeriod();

	updateGenerators();
	updateScaleSizes();
	updatePGLabel();

	ensureColourTablesSized();
	circle->updatePeriod();

	if (sendNotification)
		listeners.call(&ScaleStructureComponent::Listener::scaleStructurePeriodChanged, periodSelected);
}

void ScaleStructureComponent::onPeriodFactorChange(int factorIndexIn)
{
	DBG("\nSSC: Period factor index set to: " + String(factorIndexIn));
	scaleStructure.setAll(scaleStructure.getPeriod(), -1, -1, generatorOffset, factorIndexIn);
	periodFactorSelected = scaleStructure.getPeriodFactorIndex();
	generatorOffset = scaleStructure.getGeneratorOffset();

	periodCents = log2f(periodRatio) * 1200 / scaleStructure.getPeriodFactor();

	updateGenerators();
	updateScaleSizes();
	updateOffsetLabel();
	updatePGLabel();
	updateLsLabel();
	circle->updateGenerator();

	listeners.call(&ScaleStructureComponent::Listener::scaleStructureChanged);
}

void ScaleStructureComponent::onGeneratorChange(bool sendNotifcation)
{

}

void ScaleStructureComponent::onScaleSizeChange(bool sendNotification)
{

}

void ScaleStructureComponent::updatePGLabel()
{
	generatorValueLbl->setText(String(roundToNDecimals(generatorCents, 3)) + "c | " + String(roundToNDecimals(periodCents, 3)) + "c", dontSendNotification);
}

void ScaleStructureComponent::updateLsLabel()
{
	stepSizePatternLbl->setText(scaleStructure.getIntervalSteps(), dontSendNotification);
}

void ScaleStructureComponent::updateOffsetLabel()
{
	offsetLabel->setText("Offset\n" + String(scaleStructure.getGeneratorOffset()), dontSendNotification);
}

void ScaleStructureComponent::addListener(Listener* listenerIn)
{
	listeners.add(listenerIn);
}

void ScaleStructureComponent::removeListener(Listener* listenerIn)
{
	listeners.remove(listenerIn);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ScaleStructureComponent"
                 componentName="" parentClasses="public Component, private NumberSelector::Listener, private GroupingCircle::Listener, private Button::Listener"
                 constructorParams="ScaleStructure&amp; scaleStructureIn, Array&lt;Colour&gt;&amp; colourTableIn"
                 variableInitialisers="scaleStructure(scaleStructureIn), colourTable(colourTableIn)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="600">
  <BACKGROUND backgroundColour="ffffff"/>
  <GENERICCOMPONENT name="circleComponent" id="ec9c5dc09c2f91cf" memberName="circleComponent"
                    virtualName="" explicitFocusOrder="0" pos="0 0 100% 100%" class="GroupingCircle"
                    params="scaleStructure, colourTable"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

