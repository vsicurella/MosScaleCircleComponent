/*
  ==============================================================================

   This file is part of the mos_scale_circle JUCE module.
   A reusable control for building scales using the Moment-of-Symmetry (MOS)
   approach, rendered as an interactive "scale circle" / grouping wheel.

  ==============================================================================

  BEGIN_JUCE_MODULE_DECLARATION

   ID:                 mos_scale_circle
   vendor:             vincenzosicurella
   version:            1.0.0
   name:               MOS Scale Circle
   description:        Interactive MOS scale-circle control with a dual-mode (by-group / by-degree) colour API and ValueTree-backed master/follower parameter sync.
   website:            https://github.com/vsicurella
   license:            GPLv3
   minimumCppStandard: 17
   dependencies:       juce_core juce_data_structures juce_events juce_graphics juce_gui_basics juce_gui_extra

  END_JUCE_MODULE_DECLARATION

  ==============================================================================
*/

#pragma once
#define MOS_SCALE_CIRCLE_H_INCLUDED

//==============================================================================
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// The component sources were written against an auto-generated JuceHeader.h that
// brought the juce namespace into scope. Preserve that so the (unqualified) code
// in this module compiles unchanged.
using namespace juce;

//==============================================================================
// Public headers, included in dependency order. Each header pulls in its own
// siblings, so JUCE must already be visible (it is, from the includes above).
#include "Common.h"
#include "Symmetry.h"
#include "ScaleStructureIDs.h"
#include "ScaleStructure.h"
#include "NoteNames.h"
#include "GroupHandle.h"
#include "NumberSelector.h"
#include "TransparentDropDown.h"
#include "GroupingCircle.h"
#include "ScaleStructureComponent.h"
