/*
  ==============================================================================

    GroupingCircle.cpp
    Created: 26 Apr 2020 4:59:12pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "GroupingCircle.h"

//==============================================================================
GroupingCircle::GroupingCircle(const ScaleStructure& structureIn)
	:	scaleStructure(structureIn)
{

}

void GroupingCircle::setColourPickerLauncher(ColourPickerLauncher launcherIn)
{
	colourPickerLauncher = std::move(launcherIn);
}

GroupingCircle::~GroupingCircle()
{
	listeners.clear();
}

float GroupingCircle::getInnerRadius() const
{
	return degreeInnerRadius;
}

float GroupingCircle::getMiddleRadius() const
{
	return degreeOuterRadius;
}

float GroupingCircle::getOuterRadius() const
{
	return groupOuterRadius;
}

Rectangle<float> GroupingCircle::getInnerCircleBounds() const
{
	return degreeInnerCircleBounds;
}

Rectangle<float> GroupingCircle::getMiddleCircleBounds() const
{
	return groupInnerCircleBounds;
}

Rectangle<float> GroupingCircle::getOuterCircleBounds() const
{
	return groupOuterCircleBounds;
}

Point<float> GroupingCircle::getCenter() const
{
	return center;
}

Point<float> GroupingCircle::getFloatPointFromCenter(float radius, float angle) const
{
	return center.getPointOnCircumference(radius, angle);

}

Point<int> GroupingCircle::getIntPointFromCenter(float radius, float angle) const
{
	return getFloatPointFromCenter(radius, angle).toInt();
}

void GroupingCircle::updatePeriod()
{
	DBG("CIRCLE: Updating Period.");

	removeAllChildren();

	degreeLabels.clear();

	for (int i = 0; i < scaleStructure.getPeriod(); i++)
	{
		TextEditor* l = degreeLabels.add(new TextEditor());
		l->setJustification(Justification::centred);
		l->setReadOnly(true);
		l->setInterceptsMouseClicks(false, false);
		l->setColour(TextEditor::ColourIds::backgroundColourId, Colour());
		l->setColour(TextEditor::ColourIds::outlineColourId, Colour());
		addAndMakeVisible(l);
	}

	degreeSectorMouseOver = -1;
}

void GroupingCircle::updateGenerator()
{
	DBG("CIRCLE: Updating degree ring.");
	cancelDegreeMods();

	groupSizes = scaleStructure.getDegreeGroupSizes();
	groupChain = scaleStructure.getGroupChain();
	chromaAlterations = scaleStructure.getChromaAlterations();
	degreeAlterations = scaleStructure.getDegreeAlterations();

	DBG("\t" + arrayToString(groupChain));

	// Set up group size labels
	groupSizeLabels.clear();
	for (int i = 0; i < groupSizes.size(); i++)
	{
		Label* l = groupSizeLabels.add(new Label());
		l->setJustificationType(Justification::centred);
		l->setText(String(groupSizes[i]), dontSendNotification);
		l->setInterceptsMouseClicks(false, false);
		//l->setColour(Label::ColourIds::outlineColourId, Colours::white);

		// First one will be replaced by scaleSizeSelector
		if (i > 0)
			addChildComponent(l);
	}

	// Set up group handles
	groupHandles.clear();
	highlightedEdgeLines.clear();
	int degIndex = 0;
	for (int i = 1; i < groupSizes.size(); i++)
	{
		degIndex += groupSizes[i - 1];

		groupHandles.add(new GroupHandle(i, degIndex, true, true));
		groupHandles.add(new GroupHandle(i, degIndex + groupSizes[i], true, false));

		if (i > 1)
		{
			groupHandles.add(new GroupHandle(i, degIndex, false, false));
		}
	}

	highlightedEdgeLines.resize(groupSizes.size());

	groupSectorMouseOver = -1;

	refreshGroupLabelVisibility();

	resized();
	repaint();
}

void GroupingCircle::setNoteNameSystem(NoteNames* noteNamesIn)
{
	noteNames = noteNamesIn;
}

void GroupingCircle::setShowNoteLabels(bool showLabels)
{
	showNoteNameLabels = showLabels;
	resized();
	repaint();
}

void GroupingCircle::setAlwaysShowGroupNumbers(bool shouldShow)
{
	alwaysShowGroupNumbers = shouldShow;
	refreshGroupLabelVisibility();
	repaint();
}

void GroupingCircle::setHighlightOnMouseOver(bool shouldHighlight)
{
	highlightOnMouseOver = shouldHighlight;
	repaint();
}

void GroupingCircle::setHighlightShowsGroupNumber(bool shouldShow)
{
	highlightShowsGroupNumber = shouldShow;
	refreshGroupLabelVisibility();
	repaint();
}

void GroupingCircle::setShowGroupResizeControls(bool shouldShow)
{
	showGroupResizeControls = shouldShow;
	if (!shouldShow)
		handleMouseOver = -1;
	repaint();
}

void GroupingCircle::setShowGroups(bool shouldShow)
{
	showGroups = shouldShow;
	refreshGroupLabelVisibility();
	resized();   // recompute radii so the degree ring fills the outer area when groups are hidden
	repaint();
}

void GroupingCircle::refreshGroupLabelVisibility()
{
	for (int i = 1; i < groupSizeLabels.size(); i++)
	{
		const bool visible = showGroups
			&& (alwaysShowGroupNumbers
				|| (highlightShowsGroupNumber && i == groupSectorMouseOver));
		groupSizeLabels[i]->setVisible(visible);
	}
}

void GroupingCircle::paint (Graphics& g)
{
	g.fillAll(Colour());

	// Draw outline
	g.setColour(Colours::black);

	solidStroke.setStrokeThickness(2.0f);

	Colour groupColour;
	Colour degreeColour;
	int degIndex = 0;
	for (int i = 0; i < groupArcPaths.size(); i++)
	{
		// Resolve the group colour (also used as the fallback for degrees in this group).
		Path& groupPath = groupArcPaths.getReference(i);
		groupColour = scaleStructure.getGroupColour(i);

		if (groupColour.isTransparent())
			groupColour = Colours::lightgrey;

		if (highlightOnMouseOver && i == groupSectorMouseOver && !handleBeingDragged)
			groupColour = groupColour.contrasting(highlightContrastRatio);

		// Draw the group ring only when groups are shown.
		if (showGroups)
		{
			g.setColour(groupColour);
			g.fillPath(groupPath);

			g.setColour(Colours::black);
			g.strokePath(groupPath, solidStroke);

			groupSizeLabels[i]->setColour(Label::ColourIds::textColourId, groupColour.contrasting(labelContrastRatio));
		}

		// Draw degrees
		for (int d = 0; d < groupSizes[i]; d++)
		{
			// Resolve the degree's colour (handles both ByGroup and ByDegree modes);
			// fall back to the group colour if unresolved.
			degreeColour = scaleStructure.getDegreeColour(groupChain[degIndex]);
			if (degreeColour.isTransparent())
				degreeColour = groupColour;

			if (degreeIndexToMod > -1 && isDegreeSectorIndexModCandidate(degIndex))
			{
				// Maybe outline instead of recolor
				degreeColour = groupColour.contrasting(Colours::mediumvioletred, 1.0f / 3);
			}

			if (highlightOnMouseOver && degIndex == degreeSectorMouseOver && !handleBeingDragged)
				degreeColour = degreeColour.contrasting(highlightContrastRatio);

			Path& degreePath = degreeArcPaths.getReference(degIndex);
			
			g.setColour(degreeColour.brighter());
			g.fillPath(degreePath);

			g.setColour(degreeColour.darker());
			g.strokePath(degreePath, solidStroke);

			degreeLabels[degIndex]->applyColourToAllText(degreeColour.contrasting(labelContrastRatio));
			degIndex++;
		}
	}

	if (showGroups && showGroupResizeControls
		&& (handleBeingDragged || (mouseRadius >= degreeOuterRadius && mouseRadius < groupOuterRadius)))
	{
		// Draw edge handles
		for (int i = 0; i < groupHandles.size(); i++)
		{
			GroupHandle* handle = groupHandles.getUnchecked(i);
			g.setColour(handle->getColour());
			g.fillPath(handle->getPath());
		}
	}

	float dashPattern[2] = {
		(groupRingWidth + degreeRingWidth) / 20.0f,
		(groupRingWidth + degreeRingWidth) / 20.0f
	};

	// Highlight degree edges
	int edgePath = 0;
	for (auto index : highlightedDegreeEdges)
	{
		Colour edgeBase = scaleStructure.getGroupColour(scaleStructure.getGroupOfDegreeIndex(index));
		if (edgeBase.isTransparent())
			edgeBase = Colours::lightgrey;
		g.setColour(edgeBase.contrasting(0.75f));
		g.drawDashedLine(highlightedEdgeLines[edgePath], dashPattern, 2, 2.0f);

		edgePath++;
	}

	// Draw highlighted dragged sections
	if (handleBeingDragged)
	{
		if (abs(handleDragAmt) > 0)
		{
			Colour highlightColour;
			if (highlightedDegreeEdges.indexOf(handleDraggedToDegIndex) >= 0)
				highlightColour = handleBeingDragged->getColour().contrasting(0.5f);
			else
				highlightColour = handleBeingDragged->getColour().contrasting(Colours::lightgrey, 0.1f);

			highlightColour = highlightColour.withAlpha(0.5f);
			g.setColour(highlightColour);
			g.fillPath(handleDragHighlight);

			if (scaleStructure.isRetainingSymmetry())
			{
				g.fillPath(handleDragSymHighlight);
			}
		}
	}
}

void GroupingCircle::resized()
{
	center = Point<float>(getWidth() / 2.0f, getHeight() / 2.0f);

	// When the group ring is hidden, collapse its width so the degree ring fills the outer area,
	// and thicken the degree arcs by ~33% to use the freed space.
	groupRingWidth = showGroups ? getWidth() * groupWidthRatio : 0.0f;
	degreeRingWidth = getWidth() * degreeWidthRatio * (showGroups ? 1.0f : 1.33f);

	groupOuterRadius = getWidth() * borderRatio / 2.0f;
	degreeOuterRadius = groupOuterRadius - groupRingWidth;
	degreeInnerRadius = degreeOuterRadius - degreeRingWidth;

	groupMiddleRadius = (groupOuterRadius + degreeOuterRadius) / 2.0f;
	degreeMiddleRadius = (degreeOuterRadius + degreeInnerRadius) / 2.0f;

	handlePlacementRadius = (groupOuterRadius - degreeOuterRadius) * 0.75f + degreeOuterRadius;

	groupOuterCircleBounds = getBounds().toFloat().reduced(proportionOfWidth(1 - borderRatio));
	groupInnerCircleBounds = groupOuterCircleBounds.reduced(groupRingWidth);
	degreeInnerCircleBounds = groupInnerCircleBounds.reduced(degreeRingWidth);
	
	angleIncrement = 2 * double_Pi / groupChain.size();
	angleHalf = angleIncrement / 2.0f;

	handleDragThreshold = angleIncrement * 2.0 / 3.0f;
	handleDotRadius = groupRingWidth / 25.0f;

	// determine circle offset, based off of middle angle of first degree group
	circleOffset = groupSizes[0] * angleIncrement / 2.0f;

	int groupIndex = 0;
	int groupDegreesPassed = 0;

	float degreeLabelSize = jmin(degreeRingWidth, float_Tau * degreeMiddleRadius / degreeLabels.size()) * sectorLabelSizeRatio;
	float groupLabelSize = groupRingWidth * sectorLabelSizeRatio;

	float angle, angleTo, groupAngleFrom = -circleOffset; 
	float degLabelAngle, groupLabelAngle;

	Path degreePath, groupPath;
	TextEditor* degreeLabel;
	String degreeText;
	int degree;
	Point<int> alteration;

	Label* groupLabel;
	float degreeLabelWidth, groupLabelWidth;

	// Resize group & degree arc sections
	groupArcPaths.clear();
	degreeArcPaths.clear();
	for (int i = 0; i < degreeLabels.size(); i++)
	{
		angle = angleIncrement * i - circleOffset;
		angleTo = angle + angleIncrement;
	
		// draw arc sections
		degreePath = Path();
		addAnnulusSector(degreePath, groupInnerCircleBounds, degreeInnerCircleBounds, angle, angleTo);
		degreeArcPaths.add(degreePath);

		// find label size
		degreeLabel = degreeLabels[i];
		degreeLabel->clear();
		degreeLabel->setFont(Font().withHeight(degreeLabelSize));

		// fill labels with text
		degree = groupChain[i];

		if (showNoteNameLabels && noteNames)
		{ 
			degreeLabel->setFont(noteNames->getNoteNameFont());
			degreeLabel->insertTextAtCaret(noteNames->getChainIndexName(i));
		}
		else
		{
			degreeLabel->insertTextAtCaret(String(degree));

			alteration = degreeAlterations[degree];
			if (alteration.x >= 0 && alteration.y != 0)
			{
				degreeText = "";
				if (alteration.y > 0)
					degreeText += "+";

				degreeText += String(alteration.y);
				degreeLabel->setFont(Font(round(degreeLabelSize / 4.0f)));
				degreeLabel->insertTextAtCaret(degreeText);
			}
		}

		degreeLabelWidth = degreeLabel->getTextWidth();
		degreeLabel->setSize(jmax(degreeLabelWidth, degreeLabelSize), degreeLabelSize);

		degLabelAngle =  angleTo - angleHalf - float_HalfPi;

		degreeLabel->setCentrePosition(Point<int>(
			round(center.x + cosf(degLabelAngle) * degreeMiddleRadius),
			round(center.y + sinf(degLabelAngle) * degreeMiddleRadius)
		));

		int groupSize = groupSizes[groupIndex];
		if ((i + 1) - groupDegreesPassed >= groupSize && groupSize > 0)
		{
			groupPath = Path();
			addAnnulusSector(groupPath, groupOuterCircleBounds, groupInnerCircleBounds, groupAngleFrom, angleTo);
			groupArcPaths.add(groupPath);

			groupLabelAngle = (groupAngleFrom + angleTo) / 2.0f - float_HalfPi;

			groupLabel = groupSizeLabels[groupIndex];
			groupLabel->setFont(Font().withHeight(groupLabelSize));
			groupLabelWidth = groupLabel->getFont().getStringWidthFloat(groupLabel->getText());
			groupLabel->setSize(jmax(groupLabelWidth, groupLabelSize), groupLabelSize);

			groupLabel->setCentrePosition(Point<int>(
				round(center.x + cosf(groupLabelAngle) * groupMiddleRadius),
				round(center.y + sinf(groupLabelAngle) * groupMiddleRadius)
			));

			groupDegreesPassed += groupSize;
			groupIndex++;
			groupSize = groupSizes[groupIndex];

			groupAngleFrom = angleTo;
		}
	}

	float thetaOffset = -circleOffset - float_HalfPi;
	float outerToInnerRatio = degreeInnerRadius > 0 ? groupOuterRadius / degreeInnerRadius : 0;

	// Resize group handles
	for (int i = 0; i < groupHandles.size(); i++)
	{
		GroupHandle* handle = groupHandles.getUnchecked(i);
		float theta = angleIncrement * handle->getDegreeIndex() + thetaOffset;

		if (handle->addsGroupWhenDragged())
		{
			if (handle->isDraggingClockwise())
				handle->setPosition(Point<float>(theta + handleDotAngRatio, handlePlacementRadius), center);
			
			else
				handle->setPosition(Point<float>(theta - handleDotAngRatio, handlePlacementRadius), center);

			handle->setSize(handleDotRadius);
		}
		else
		{
			handle->setPosition(Point<float>(theta, degreeInnerRadius), center);
			handle->setSize(outerToInnerRatio);
		}
	}

	// Resize degree edge lines if groups are being resized
	if (highlightedDegreeEdges.size() > 0)
	{
		highlightedEdgeLines.clear();

		float length = outerToInnerRatio;
		int index;
		Line<float> line;
		
		for (int i = 0; i < highlightedDegreeEdges.size(); i++)
		{
			index = highlightedDegreeEdges[i];
			line = GroupHandle::getGroupEdgeLine(center, Point<float>(angleIncrement * index + thetaOffset, degreeInnerRadius), length);
			highlightedEdgeLines.add(line);
		}
	}

	// Resize highlights of dragged sections
	if (handleBeingDragged)
	{
		if (abs(handleDragAmt) > 0)
		{
			int dragFrom = handleBeingDragged->getDegreeIndex();
			int dragTo = handleDraggedToDegIndex;

			handleDragHighlight.clear();
			addAnnulusSector(
				handleDragHighlight,
				groupOuterCircleBounds,
				degreeInnerCircleBounds,
				dragFrom * angleIncrement - circleOffset,
				dragTo * angleIncrement - circleOffset
			);

			if (scaleStructure.isRetainingSymmetry())
			{
				int symGroup = scaleStructure.getSymmetricGroup(handleBeingDragged->getGroupIndex());
				dragFrom = groupChain.indexOf(scaleStructure.getDegreeGroupings()[symGroup][0]);
				
				if (!handleBeingDragged->addsGroupWhenDragged() || handleBeingDragged->isDraggingClockwise())
					dragFrom += groupSizes[symGroup];

				dragTo = dragFrom - handleDragAmt;
				handleDragSymHighlight.clear();
				addAnnulusSector(
					handleDragSymHighlight,
					degreeInnerCircleBounds,
					groupOuterCircleBounds,
					dragFrom * angleIncrement - circleOffset,
					dragTo * angleIncrement - circleOffset
				);
			}
		}
	}
	else
	{
		handleDragHighlight.clear();
		handleDragSymHighlight.clear();
	}
}

void GroupingCircle::mouseMove(const MouseEvent& event)
{
	mouseRadius = event.position.getDistanceFrom(center);
	bool dirty = false;

	// If mouse is not in rings, remove highlights
	if (mouseRadius < degreeInnerRadius || mouseRadius > groupOuterRadius)
	{
		lastDegreeSectorMouseIn = degreeSectorMouseOver;
		degreeSectorMouseOver = -1;

		lastGroupSectorMouseIn = groupSectorMouseOver;
		groupSectorMouseOver = -1;

		if (handleMouseOver > -1)
		{
			groupHandles[handleMouseOver]->setMouseOver(false);
			handleMouseOver = -1;
		}
		
		dirty = true;
	}

	else
	{
		float angle = getNormalizedMouseAngle(event);
		int degreeIndex = degreeSectorOfAngle(angle);

		// Check Degree Sectors
		if (mouseRadius < degreeOuterRadius)
		{
			if (degreeSectorMouseOver != degreeIndex)
			{
				lastDegreeSectorMouseIn = degreeSectorMouseOver;
				degreeSectorMouseOver = degreeIndex;
				dirty = true;
			}

			if (groupSectorMouseOver > -1)
			{
				lastGroupSectorMouseIn = groupSectorMouseOver;
				groupSectorMouseOver = -1;
				dirty = true;
			}

			if (handleMouseOver > -1)
			{
				handleMouseOver = -1;
				dirty = true;
			}
		}

		// Check Group Sectors
		else if (mouseRadius < groupOuterRadius)
		{
			int groupIndex = mouseInGroupSector(degreeIndex);

			if (groupSectorMouseOver != groupIndex)
			{
				lastGroupSectorMouseIn = groupSectorMouseOver;
				groupSectorMouseOver = groupIndex;
				dirty = true;
			}

			if (degreeSectorMouseOver > -1)
			{
				lastDegreeSectorMouseIn = degreeSectorMouseOver;
				degreeSectorMouseOver = -1;
				dirty = true;
			}

			// check if over a handle
			if (showGroupResizeControls)
			{
				int handleIndex;
				for (handleIndex = 0; handleIndex < groupHandles.size(); handleIndex++)
				{
					GroupHandle* handle = groupHandles.getUnchecked(handleIndex);

					// TODO: improve detecting?
					if (handle->isMouseOver(event))
					{
						handleMouseOver = handleIndex;
						dirty = true;
						break;
					}
				}

				if (handleIndex >= groupHandles.size())
				{
					handleMouseOver = -1;
					dirty = true;
				}
			}

		}
	}
	
	if (handleMouseOver > -1)
	{
		if (!groupHandles[handleMouseOver]->isMouseOver(event))
		{
			handleMouseOver = -1;
			dirty = true;
		}
	}

	// TODO: implement ring sectors as components so only certain ones need to be repainted
	if (dirty)
	{
		// Reveal the hovered group's number label when that option is enabled.
		if (highlightShowsGroupNumber)
			refreshGroupLabelVisibility();

		repaint();
	}
}

void GroupingCircle::mouseDown(const MouseEvent& event)
{
	bool cancelMods = true;

	// If mouse is on a group or degree section
	if (mouseRadius > degreeInnerRadius && mouseRadius < groupOuterRadius)
	{
		// If mouse on a degree
		if (mouseRadius < degreeOuterRadius)
		{
			degreeSectorMouseOver = degreeSectorOfAngle(getNormalizedMouseAngle(event));

			// Helpers for offset manipulation
			lastDegClicked = degreeSectorMouseOver;
			lastOffsetOnClick = scaleStructure.getGeneratorOffset();

			int degree = groupChain[degreeSectorMouseOver];
			// Show menu
			if (event.mods.isRightButtonDown())
			{
				PopupMenu::Options options = PopupMenu::Options().withMaximumNumColumns(1);

				int degreeIndexMouseOn = degreeSectorMouseOver;
				bool& hideMods = cancelMods;
				bool& showNames = showNoteNameLabels;
								
				auto selectionCallback = [this](int degIndex) {
					degreeToModSelectedCallback(degIndex);
				};
				
				auto alterationCallback = [this](int degIndex, Point<int> alteration) {
					listeners.call(&Listener::degreeIndexAltered, degIndex, alteration);
					updateGenerator();
				};

				auto resetCallback = [this]() {
					listeners.call(&Listener::allModificationsReset);
					updateGenerator();
				};

				degreeMenu.clear();
				degreeMenu.addItem("Show modifiable degrees", true, false, [selectionCallback, degreeIndexMouseOn, &hideMods](void) {
					selectionCallback(degreeIndexMouseOn);
					hideMods = false;
				});

				degreeMenu.addItem("Reset degree modifications", isDegreeSectorIndexAltered(degreeSectorMouseOver), false, [degreeIndexMouseOn, alterationCallback](void) {
					alterationCallback(degreeIndexMouseOn, Point<int>(-1, 0));
				});

				degreeMenu.addItem("Reset all modifications", true, false, resetCallback);

				degreeMenu.addItem("Toggle Note Names", true, showNoteNameLabels, [&showNames](void) {
					showNames = !showNames;
				});

				// Per-degree colour assignment is only meaningful in ByDegree mode.
				if (scaleStructure.getColourMode() == ScaleStructure::ColourMode::ByDegree)
				{
					degreeMenu.addSeparator();
					degreeMenu.addItem("Assign degree colour...", true, false, [this, degree]() {
						openColourPickerForDegree(degree);
					});
					degreeMenu.addItem("Reset degree colour", scaleStructure.hasDegreeColourOverride(degree), false, [this, degree]() {
						listeners.call(&Listener::degreeColourChanged, degree, Colour());
						repaint();
					});
				}

				degreeMenu.showMenuAsync(options);
			}

			// Mouse left-clicked on mod candidate
			else if (isDegreeSectorIndexModCandidate(degreeSectorMouseOver))
			{
				listeners.call(&Listener::degreeIndexAltered, degreeIndexToMod, degreeModCandidates[degree]);
				updateGenerator();
			}
		}

		// If mouse in a group area
		else if (mouseRadius < groupOuterRadius)
		{
			// Right-click on a group sector: offer to assign that group's colour.
			if (event.mods.isRightButtonDown())
			{
				int degSector = degreeSectorOfAngle(getNormalizedMouseAngle(event));
				int groupIndex = (degSector >= 0) ? mouseInGroupSector(degSector) : -1;
				if (groupIndex >= 0)
				{
					groupMenu.clear();
					groupMenu.addItem("Assign group colour...", true, false, [this, groupIndex]() {
						openColourPickerForGroup(groupIndex);
					});
					groupMenu.showMenuAsync(PopupMenu::Options().withMaximumNumColumns(1));
				}
			}

			// TODO: move most calculations to ScaleStructure class
			// Show possible degrees to drag handle to
			else if (handleMouseOver > -1)
			{
				handleBeingDragged = groupHandles[handleMouseOver];

				int groupIndex = handleBeingDragged->getGroupIndex();
				int groupSize = groupSizes[groupIndex];

				Array<int> newValidSizes;
				Array<Point<int>> validIndicies;
				Point<int> indicies;
				int size, newDegreeEdge;
				highlightedDegreeEdges.clear();

				if (handleBeingDragged->addsGroupWhenDragged())
				{
					validIndicies = scaleStructure.findIndiciesForGroupSplitting(handleBeingDragged->getGroupIndex(), !handleBeingDragged->isDraggingClockwise());
					for (int i = 0; i < validIndicies.size(); i++)
					{
						indicies = validIndicies[i];

						// TODO: improve
						if (indicies.x > -1)
							highlightedDegreeEdges.add(indicies.x);
						if (indicies.y > -1)
							highlightedDegreeEdges.add(indicies.y);
					}
				}

				else
				{
					validIndicies = scaleStructure.findIndiciesForGroupResizing(handleBeingDragged->getGroupIndex());
					for (int i = 0; i < validIndicies.size(); i++)
					{
						indicies = validIndicies[i];

						// TODO: improve
						if (indicies.x > -1)
							highlightedDegreeEdges.add(indicies.x);
						if (indicies.y > -1)
							highlightedDegreeEdges.add(indicies.y);
						
					}
				}

				resized();

				// Show group size labels during drag
				for (int i = 1; i < groupSizeLabels.size(); i++)
					groupSizeLabels[i]->setVisible(true);

				DBG("CIRCLE: Handle of Degree Index " + String(handleBeingDragged->getDegreeIndex()) + " grabbed");
				//DBG("CIRCLE: Handle grabbed. Available degree edges:");
				//String dbgstr;
				//for (auto hdi : highlightedDegreeEdges)
				//{
				//	dbgstr = "\t" + String(hdi);
				//	DBG(dbgstr);
				//}
			}
		}
	}

	if (cancelMods)
	{
		cancelDegreeMods();
		repaint();
	}
}

void GroupingCircle::mouseDrag(const MouseEvent& event)
{
	mouseDownRadius = event.mouseDownPosition.getDistanceFrom(center);
	mouseRadius = event.position.getDistanceFrom(center);

	bool degreeSectorChanged = false;
	float mouseAngle = getNormalizedMouseAngle(event);
	degreeSectorMouseOver = degreeSectorOfAngle(mouseAngle);
	if (degreeSectorMouseOver != degreeSectorMouseOver)
	{
		lastDegreeSectorMouseIn = degreeSectorMouseOver;
		degreeSectorChanged = true;
	}

	bool dirty = false;

	if (mouseDownRadius >= degreeInnerRadius)
	{
		// Dragging a degree
		if (mouseDownRadius < degreeOuterRadius)
		{
			int period = scaleStructure.getPeriod();
			int offsetFromRoot = lastDegClicked > (period / 2)
				? lastDegClicked - period + lastOffsetOnClick
				: lastDegClicked - lastOffsetOnClick;

			int rootPos = (degreeSectorMouseOver - offsetFromRoot) % period;

			if (rootPos >= 0 && rootPos <= (groupSizes[0] / scaleStructure.getPeriodFactor()) - 1)
			{
				if (rootPos != scaleStructure.getGeneratorOffset())
				{
					listeners.call(&Listener::offsetChanged, rootPos);

					DBG("Moved by " + String(rootPos - lastOffsetOnClick) + "\tNew offset: " + String(rootPos));
					dirty = true;
				}
			}
		}

		// In group sectors
		else if (mouseDownRadius <= groupOuterRadius)
		{
			// Dragging a handle
			if (handleBeingDragged)
			{
				float degreesMouseMoved = mouseAngle / angleIncrement - handleBeingDragged->getDegreeIndex();
				if (degreesMouseMoved < -scaleStructure.getPeriod() / 2)
					degreesMouseMoved = scaleStructure.getPeriod() + degreesMouseMoved;

				if (handleDraggedToDegIndex != degreeSectorMouseOver)
				{
					// Set drag direction of edge handles
					if (!handleBeingDragged->addsGroupWhenDragged())
					{
						if (degreesMouseMoved > 0)
						{
							handleBeingDragged->setDraggingClockwise(true);
						}
						else
						{
							handleBeingDragged->setDraggingClockwise(false);
						}
					}
				}

				// Basically an offset version of calculating degreeSectorMouseOver
				// using the handleDragThreshold as drag amt trigger

				float angDistance = degreesMouseMoved - (int)(degreesMouseMoved);
				if (!handleBeingDragged->isDraggingClockwise())
					angDistance = angDistance - (int)(angDistance);

				angDistance = abs(angDistance * angleIncrement);

				handleDragAmt = degreesMouseMoved;
				
				if (angDistance >= handleDragThreshold)
				{
					handleDragAmt += (abs(degreesMouseMoved) / degreesMouseMoved);
				}

				int groupIndex = handleBeingDragged->getGroupIndex();
				int adjGroup = groupIndex - 1; // for edge handles only
				int dragLimit;

				// TODO: some abstraction of this (since it's reused in this class and also a bit in ScaleStructure)
				if (handleBeingDragged->addsGroupWhenDragged() || handleBeingDragged->isDraggingClockwise())
				{
					dragLimit = groupSizes[groupIndex];

					if (groupIndex == scaleStructure.getSymmetricGroup(groupIndex))
						dragLimit /= 2;
				}
				else
				{
					dragLimit = groupSizes[adjGroup];

					if (adjGroup == scaleStructure.getSymmetricGroup(adjGroup))
						dragLimit /= 2;
				}
					
				if (abs(handleDragAmt) <= dragLimit)
				{
					lastDraggedIndex = handleDraggedToDegIndex;
					handleDraggedToDegIndex = handleBeingDragged->getDegreeIndex() + handleDragAmt;

					// Update group size labels to preview new sizes
					for (int i = 0; i < groupSizeLabels.size(); i++)
						groupSizeLabels[i]->setText(String(groupSizes[i]), dontSendNotification);

					if (handleDragAmt != 0)
					{
						int absAmt = abs(handleDragAmt);

						if (handleBeingDragged->addsGroupWhenDragged())
						{
							// Split: the group being split shrinks
							int newSize = groupSizes[groupIndex] - absAmt;

							if (scaleStructure.isRetainingSymmetry())
							{
								int symGroup = scaleStructure.getSymmetricGroup(groupIndex);
								if (symGroup == groupIndex)
									newSize -= absAmt; // Self-symmetric: double the effect
								else
									groupSizeLabels[symGroup]->setText(
										String(groupSizes[symGroup] - absAmt), dontSendNotification);
							}

							groupSizeLabels[groupIndex]->setText(String(newSize), dontSendNotification);
						}
						else
						{
							// Resize: one group shrinks, the other grows
							int newGroupSize = groupSizes[groupIndex] - handleDragAmt;
							int newAdjSize = groupSizes[adjGroup] + handleDragAmt;

							if (scaleStructure.isRetainingSymmetry())
							{
								int symGroup = scaleStructure.getSymmetricGroup(groupIndex);
								int numGroups = groupSizes.size();
								int symAdj = (symGroup + 1) % numGroups;

								if (groupIndex == symAdj)
								{
									// resizeAndAdd: resize creates a new group
									int absAmt = abs(handleDragAmt);
									newGroupSize = groupSizes[groupIndex] - absAmt;
									newAdjSize = absAmt * 2;
								}
								else
								{
									// Self-symmetric groupIndex: drag affects both boundaries
									if (symGroup == groupIndex)
										newGroupSize -= handleDragAmt;
									// Self-symmetric adjGroup: symmetric drag also changes adjGroup
									else if (adjGroup == scaleStructure.getSymmetricGroup(adjGroup))
										newAdjSize += handleDragAmt;

									// Update symmetric counterpart of groupIndex
									if (symGroup != groupIndex && symGroup != adjGroup)
										groupSizeLabels[symGroup]->setText(
											String(groupSizes[symGroup] - handleDragAmt), dontSendNotification);

									// Update symmetric adjacent
									if (symAdj != groupIndex && symAdj != adjGroup)
										groupSizeLabels[symAdj]->setText(
											String(groupSizes[symAdj] + handleDragAmt), dontSendNotification);
								}
							}

							groupSizeLabels[groupIndex]->setText(String(newGroupSize), dontSendNotification);
							groupSizeLabels[adjGroup]->setText(String(newAdjSize), dontSendNotification);
						}
					}

					resized();
					repaint();
				}
			}
		}
	}

	if (dirty)
	{
		updateGenerator();
	}
}

void GroupingCircle::mouseUp(const MouseEvent& event)
{

	if (handleBeingDragged)
	{
		DBG("Handle released at AMT " + String(handleDragAmt));

		// Figure out type of change (merge, resize, split)
		if (highlightedDegreeEdges.contains(handleDraggedToDegIndex))
		{
			int groupIndex = handleBeingDragged->getGroupIndex();

			// If handle is a dot
			if (handleBeingDragged->addsGroupWhenDragged())
			{
				// For dots, the group affected is in the opposite direction of dragging
				listeners.call(&Listener::groupingSplit, groupIndex, handleDragAmt);
			}

			// If handle is an edge
			else
			{
				// Check if the new size merges the adjacent group
				bool groupsMerged = false;
				int adjGroup = groupIndex - 1;

				// TODO: some abstraction of this (since it's reused in this class and also a bit in ScaleStructure)
				if (handleBeingDragged->isDraggingClockwise())
				{
					if (groupIndex != scaleStructure.getSymmetricGroup(groupIndex) && abs(handleDragAmt) == groupSizes[groupIndex])
						groupsMerged = true;
				}
				else
				{
					if (adjGroup != scaleStructure.getSymmetricGroup(adjGroup) && abs(handleDragAmt) == groupSizes[adjGroup])
						groupsMerged = true;
				}

				// If adjacent group is consumed, do a merge
				if (groupsMerged)
					listeners.call(&Listener::groupingsMerged, groupIndex);
				else
					listeners.call(&Listener::groupingResized, groupIndex, handleDragAmt, handleBeingDragged->isDraggingClockwise());
			}

			handleBeingDragged = nullptr;
			updateGenerator();
		}

		// Restore original text after drag; visibility honours the always-show / showGroups toggles.
		for (int i = 1; i < groupSizeLabels.size(); i++)
			groupSizeLabels[i]->setText(String(groupSizes[i]), dontSendNotification);
		refreshGroupLabelVisibility();

		handleBeingDragged = nullptr;
		handleDraggedToDegIndex = -1;
		handleDragAmt = 0;
		highlightedDegreeEdges.clear();
		repaint();
	}
}

void GroupingCircle::addListener(Listener* listenerToAdd)
{
	listeners.add(listenerToAdd);
}

void GroupingCircle::removeListener(Listener* listenerToRemove)
{
	listeners.remove(listenerToRemove);
}

//==============================================================================
// Colour picker

namespace
{
	// Wraps a juce::ColourSelector for use in a CallOutBox and forwards live colour
	// changes to a callback. This is the built-in fallback when no launcher is set.
	class ColourSelectorCallout : public Component,
	                              private ChangeListener
	{
	public:
		ColourSelectorCallout(Colour initialColour, std::function<void (Colour)> onColourIn)
			: onColour(std::move(onColourIn))
		{
			selector.setCurrentColour(initialColour, dontSendNotification);
			selector.addChangeListener(this);
			addAndMakeVisible(selector);
			setSize(260, 300);
		}

		~ColourSelectorCallout() override
		{
			selector.removeChangeListener(this);
		}

		void resized() override { selector.setBounds(getLocalBounds()); }

	private:
		void changeListenerCallback(ChangeBroadcaster*) override
		{
			if (onColour)
				onColour(selector.getCurrentColour());
		}

		ColourSelector selector { ColourSelector::showColourAtTop | ColourSelector::showSliders | ColourSelector::showColourspace };
		std::function<void (Colour)> onColour;
	};
}

void GroupingCircle::openColourPickerForGroup(int groupIndex)
{
	colourTargetGroup = groupIndex;
	colourTargetDegree = -1;
	launchColourPicker();
}

void GroupingCircle::openColourPickerForDegree(int degreeIndex)
{
	colourTargetGroup = -1;
	colourTargetDegree = degreeIndex;
	launchColourPicker();
}

void GroupingCircle::launchColourPicker()
{
	// The target's current colour and section bounds (in this component's coordinates).
	Colour current = Colours::white;
	Rectangle<int> area;

	if (colourTargetDegree >= 0)
	{
		current = scaleStructure.getDegreeColour(colourTargetDegree);
		int visualIndex = groupChain.indexOf(colourTargetDegree);
		if (visualIndex >= 0 && visualIndex < degreeArcPaths.size())
			area = degreeArcPaths.getReference(visualIndex).getBounds().toNearestInt();
	}
	else if (colourTargetGroup >= 0 && colourTargetGroup < groupArcPaths.size())
	{
		current = scaleStructure.getGroupColour(colourTargetGroup);
		area = groupArcPaths.getReference(colourTargetGroup).getBounds().toNearestInt();
	}

	if (current.isTransparent())
		current = Colours::white;

	if (area.isEmpty())
		area = getLocalBounds().withSizeKeepingCentre(20, 20);

	// Capture the targets by value so the callback is robust to later state changes.
	const int targetGroup = colourTargetGroup;
	const int targetDegree = colourTargetDegree;

	auto onPicked = [this, targetGroup, targetDegree](Colour chosen)
	{
		if (targetGroup >= 0)
			listeners.call(&Listener::groupColourChanged, targetGroup, chosen);
		else if (targetDegree >= 0)
			listeners.call(&Listener::degreeColourChanged, targetDegree, chosen);

		repaint();
	};

	// Prefer the host-supplied launcher; otherwise show the built-in ColourSelector.
	if (colourPickerLauncher)
	{
		colourPickerLauncher(area, current, onPicked);
		return;
	}

	Rectangle<int> screenArea = area.translated(getScreenPosition().x, getScreenPosition().y);
	CallOutBox::launchAsynchronously(std::make_unique<ColourSelectorCallout>(current, onPicked),
	                                 screenArea, nullptr);
}

String GroupingCircle::getTooltip()
{
	return "Drag the degree ring to change the offset, drag a group edge to resize, "
	       "or right-click to assign colours.";
}

void GroupingCircle::degreeToModSelectedCallback(int degreeIndex)
{
	degreeIndexToMod = degreeIndex;
	degreeModCandidates = scaleStructure.findDegreeMods(degreeIndexToMod, -1);
	TextEditor* label = degreeLabels[degreeIndexToMod];
	label->setFont(label->getFont().withHeight(label->getHeight() / 3.0f));
	label->insertTextAtCaret("*");
	repaint();
}

void GroupingCircle::cancelDegreeMods()
{
	if (degreeIndexToMod > -1 && degreeIndexToMod < scaleStructure.getPeriod())
	{
		TextEditor* label = degreeLabels[degreeIndexToMod];
		label->setHighlightedRegion(Range<int>(label->getText().length() - 1, label->getText().length()));
		label->insertTextAtCaret(String());
		degreeIndexToMod = -1;
		degreeModCandidates.clear();
	}
	
}

float GroupingCircle::getNormalizedMouseAngle(const MouseEvent& event) const
{
	float angle = atan2f(event.position.x - center.x, center.y - event.position.y) + (groupSizes[0] / 2.0f * angleIncrement);
	if (angle < 0) angle += float_Tau;
	if (angle >= float_Tau) angle -= float_Tau;

	return angle;
}

int GroupingCircle::degreeSectorOfAngle(float angle) const
{
	return (int)(angle / angleIncrement) % groupChain.size();
}

int GroupingCircle::mouseInDegreeRingSector(const MouseEvent& event, float radiusFromCenter, float angle) const
{
	if (radiusFromCenter >= degreeInnerRadius && radiusFromCenter < degreeOuterRadius)
		return degreeSectorOfAngle(angle);

	return -1;
}

int GroupingCircle::mouseInGroupSector(int degreeIndex) const
{
	int groupIndex = 0;
	int groupSize = groupSizes[groupIndex];
	while (degreeIndex >= groupSize)
	{
		degreeIndex -= groupSize;

		if (degreeIndex < 0 || groupIndex == groupSizes.size() - 1)
			return -1;

		groupSize = groupSizes[++groupIndex];
	}
	
	return groupIndex;
}

int GroupingCircle::mouseInGroupRingSector(const MouseEvent& event, float radiusFromCenter, float angle) const
{
	if (radiusFromCenter >= degreeOuterRadius && radiusFromCenter < groupOuterRadius)
		mouseInGroupSector(degreeSectorOfAngle(angle));

	return -1;
}

int GroupingCircle::mouseInGroupRingSector(const MouseEvent& event, float radiusFromCenter, int degreeIndex) const
{
	if (radiusFromCenter >= degreeOuterRadius && radiusFromCenter < groupOuterRadius)
		mouseInGroupSector(degreeIndex);

	return -1;
}

/*
	Returns true if degree sector is a modification candidate
*/
bool GroupingCircle::isDegreeSectorIndexModCandidate(int degreeSectorIndexIn) const
{
	if (degreeSectorIndexIn >= 0 && degreeSectorIndexIn < degreeModCandidates.size())
	{
		const Point<int>& alteration = degreeModCandidates.getReference(groupChain[degreeSectorIndexIn]);
		return alteration.x >= 0 && alteration.y != 0;
	}
	return false;
}

/*
	Returns true if degree sector is altered
*/
bool GroupingCircle::isDegreeSectorIndexAltered(int degreeSectorIndexIn) const
{
	if (degreeSectorIndexIn >= 0 && degreeSectorIndexIn < degreeAlterations.size())
	{
		const Point<int>& alteration = degreeAlterations.getReference(groupChain[degreeSectorIndexIn]);
		return alteration.x >= 0 && alteration.y != 0;
	}
	return false;
}

void GroupingCircle::addArcToPath(Path& pathIn, Rectangle<float> ellipseBounds, float fromRadians, float toRadians, bool startAsNewSubPath)
{
	pathIn.addArc(
		ellipseBounds.getX(),
		ellipseBounds.getY(),
		ellipseBounds.getWidth(),
		ellipseBounds.getHeight(),
		fromRadians,
		toRadians,
		startAsNewSubPath
	);
}

void GroupingCircle::addAnnulusSector(Path& pathIn, Rectangle<float> outerBounds, Rectangle<float> innerBounds, float fromRadians, float toRadians)
{
	pathIn.addArc(outerBounds.getX(), outerBounds.getY(), outerBounds.getWidth(), outerBounds.getHeight(), fromRadians, toRadians, true);
	pathIn.addArc(innerBounds.getX(), innerBounds.getY(), innerBounds.getWidth(), innerBounds.getHeight(), toRadians, fromRadians);
	pathIn.closeSubPath();
}