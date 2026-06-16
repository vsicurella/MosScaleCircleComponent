/*
  ==============================================================================

    GroupingCircle.h
    Created: 26 Apr 2020 4:59:12pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "ScaleStructure.h"
#include "GroupHandle.h"
#include "NoteNames.h"

//==============================================================================
/*
*/
class GroupingCircle    : public Component,
                          public TooltipClient
{
public:
    GroupingCircle(const ScaleStructure& structureIn);
    ~GroupingCircle();

	// Host-supplied launcher used to assign a colour to a group or degree. The component passes
	// the target section's bounds (in this component's coordinates) and its current colour; the
	// host shows its own picker and invokes onPicked(chosen) when the user commits. When this is
	// left unset, a built-in juce::ColourSelector is shown in a CallOutBox instead.
	using ColourPickerLauncher = std::function<void (Rectangle<int> area, Colour current, std::function<void (Colour)> onPicked)>;
	void setColourPickerLauncher(ColourPickerLauncher launcherIn);

	float getInnerRadius() const;
	float getMiddleRadius() const;
	float getOuterRadius() const;

	Rectangle<float> getInnerCircleBounds() const;
	Rectangle<float> getMiddleCircleBounds() const;
	Rectangle<float> getOuterCircleBounds() const;

	Point<float> getCenter() const;
	Point<float> getFloatPointFromCenter(float radius, float angle) const;
	Point<int> getIntPointFromCenter(float radius, float angle) const;

	void updatePeriod();
	void updateGenerator();

	void setNoteNameSystem(NoteNames* noteNamesIn);
	void setShowNoteLabels(bool showLabels);

	// Display toggles (see members for defaults).
	void setAlwaysShowGroupNumbers(bool shouldShow);     // show group-size numbers even when not dragging
	void setHighlightOnMouseOver(bool shouldHighlight);  // recolour the hovered group/degree
	void setHighlightShowsGroupNumber(bool shouldShow);  // reveal the hovered group's number label
	void setShowGroupResizeControls(bool shouldShow);    // show the draggable group-edge handles
	void setShowGroups(bool shouldShow);                 // show the outer group ring (else only the degree ring)

	void degreeToModSelectedCallback(int degreeIndex);
	void cancelDegreeMods();

    void paint (Graphics&) override;
    void resized() override;

	void mouseMove(const MouseEvent& event) override;
	void mouseDown(const MouseEvent& event) override;
	void mouseDrag(const MouseEvent& event) override;
	void mouseUp(const MouseEvent& event) override;

	// TooltipClient - hints that the rings can be dragged / right-clicked.
	String getTooltip() override;


	class Listener
	{
	public:
		virtual ~Listener() {}

		virtual void offsetChanged(int newOffset) = 0;

		virtual void degreeIndexAltered(int degreeIndex, Point<int> alteration) = 0;
		virtual void allModificationsReset() = 0;

		virtual void groupingSplit(int groupIndex, int sizeChangeAmount) = 0;
		virtual void groupingResized(int groupIndex, int sizeChangeAmount, bool draggedClockwise) = 0;
		virtual void groupingsMerged(int groupIndex) = 0;

		// Called when the user assigns a colour to a degree group (or a single degree) from the wheel.
		// Pass a transparent colour to clear a per-degree override.
		virtual void groupColourChanged(int groupIndex, Colour newColour) {}
		virtual void degreeColourChanged(int degreeIndex, Colour newColour) {}
	};

	void addListener(Listener* listenerToAdd);
	void removeListener(Listener* listenerToRemove);

public:

	static void addArcToPath(Path& pathIn, Rectangle<float> ellipseBounds, float fromRadians, float toRadians, bool startAsNewSubPath = false);
	static void addAnnulusSector(Path& pathIn, Rectangle<float> outerBounds, Rectangle<float> innerBounds, float fromRadians, float toRadians);

protected:
	ListenerList<Listener> listeners;

private:
	OwnedArray<TextEditor> degreeLabels;
	OwnedArray<Label> groupSizeLabels;

	const ScaleStructure& scaleStructure;

	// The groups of scale degree arrays
	Array<Array<int>> degreeGroupings;

	// The chain of scale degrees by stacking the chosen generator
	Array<int> groupChain;

	// Starting from the top (scale size selected) going clockwise
	Array<int> groupSizes;

	PopupMenu groupMenu;
	PopupMenu degreeMenu;

	// Colour assignment: which group/degree a launched colour picker should write to (-1 if none).
	int colourTargetGroup = -1;
	int colourTargetDegree = -1;
	ColourPickerLauncher colourPickerLauncher;

	// Launches the host picker (or the built-in juce::ColourSelector fallback) to assign a colour
	// to a group or a single degree; the chosen colour is delivered via the listener callbacks.
	void openColourPickerForGroup(int groupIndex);
	void openColourPickerForDegree(int degreeIndex);
	void launchColourPicker();

	NoteNames* noteNames = nullptr;
	bool showNoteNameLabels = false;

	// Display toggles.
	bool alwaysShowGroupNumbers = false;
	bool highlightOnMouseOver = true;
	bool highlightShowsGroupNumber = true;
	bool showGroupResizeControls = true;
	bool showGroups = true;

	// Applies alwaysShowGroupNumbers / showGroups to the group-size labels.
	void refreshGroupLabelVisibility();

	// MODMOS functionality
	int degreeIndexToMod = -1;
	Array<Point<int>> degreeModCandidates;
	Array<Point<int>> chromaAlterations;
	Array<Point<int>> degreeAlterations;

	// Group sizing functionality
	OwnedArray<GroupHandle> groupHandles;
	int handleMouseOver = -1;
	GroupHandle* handleBeingDragged = nullptr;
	float handleDragThreshold;
	float handleDraggedToDegIndex = -1;
	float lastDraggedIndex = -1;
	int handleDragAmt = 0;

	// Refers to the most counter-clockwise edge of a degree
	Array<int> highlightedDegreeEdges; // TODO: turn this into Array<Point<int>> to differentiate symmetric edges
	Array<Line<float>> highlightedEdgeLines;
		
	const float handleDotAngRatio = MathConstants<float>::pi / 100.0f;
	float handleDotRadius;
	float handleHighlightMult = 1.5f;
	float handlePlacementRadius;
	
	// Mouse functionality
	int mouseRadius, mouseDownRadius;
	int degreeSectorMouseOver = -1;
	int groupSectorMouseOver = -1;
	int lastDegreeSectorMouseIn = -1;
	int lastGroupSectorMouseIn = -1;
	int lastDegClicked = -1;
	int lastOffsetOnClick = 0;

	// Drawing related members
	float borderRatio = 127.0f / 128.0f;
	const float groupWidthRatio = 1.0f / 10.0f;
	const float degreeWidthRatio = 1.0f / 15.0f;

	float groupOuterRadius;
	float groupMiddleRadius;

	float degreeOuterRadius;
	float degreeInnerRadius;
	float degreeMiddleRadius;

	Point<float> center;
	float circleOffset = MathConstants<float>::pi / 2.0f;

	float groupRingWidth;
	float degreeRingWidth;

	Rectangle<float> groupOuterCircleBounds;
	Rectangle<float> groupInnerCircleBounds;
	Rectangle<float> degreeInnerCircleBounds;

	double angleIncrement;
	double angleHalf;
	const float float_HalfPi = MathConstants<float>::pi / 2;
	const float float_Tau = MathConstants<float>::pi * 2;

	Array<Line<float>> radiLines;
	Array<Path> degreeArcPaths;
	Array<Path> groupArcPaths;
	PathStrokeType solidStroke = PathStrokeType(2.0f);
	PathStrokeType dashedStroke = PathStrokeType(2.0f);

	Path handleDragHighlight;
	Path handleDragSymHighlight;

	float sectorLabelSizeRatio = 0.875f;

	float highlightContrastRatio = 1.0f / 6.0f;
	float labelContrastRatio = 2.0f / 3.0f;

private:

	/*
		Returns the angle of the mouse based on the circle's center, with 0 starting at the counter-clockwise edge 
		of the first degree group's degree, and moving clockwise.
	*/
	float getNormalizedMouseAngle(const MouseEvent& event) const;

	/*
		Returns the degree index of the degree's corresponding sector that the angle is in.
	*/
	int degreeSectorOfAngle(float angle) const;

	/*
		Returns the degree index that the mouse is in if in the area of a degree ring sector.
		Returns -1 if outside of ring sector.
	*/
	int mouseInDegreeRingSector(const MouseEvent& event, float radiusFromCenter, float angle) const;

	/*
		Returns the group size index of the group's corresponding sector that the mouse is in
	*/
	int mouseInGroupSector(int degreeIndex) const;

	/*
		Returns the group size index that the mouse is in if in the area of a group size ring sector.
		Returns -1 if outside of ring sector.
	*/
	int mouseInGroupRingSector(const MouseEvent& event, float radiusFromCenter, float angle) const;
	int mouseInGroupRingSector(const MouseEvent& event, float radiusFromCenter, int degreeIndex) const;

	/*
		Returns true if degree sector is a modification candidate
	*/
	bool isDegreeSectorIndexModCandidate(int degreeSectorIndexIn) const;

	/*
		Returns true if degree sector is altered
	*/
	bool isDegreeSectorIndexAltered(int degreeSectorIndexIn) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupingCircle)
};
