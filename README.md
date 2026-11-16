# MOS Scale Circle

A reusable JUCE **module** providing an interactive "scale circle" / grouping-wheel control for
building scales with the [Moment of Symmetry (MOS)](https://en.xen.wiki/w/MOS_scale) approach.

The control lets a user choose a period, generator, scale size and offset, arrange degrees into
groups by dragging, and assign colours. It can act as the **broadcaster** of the scale parameters or as a **follower** driven by an external controller.

## Consuming the module

JUCE 6+/CMake. Add the module and link it; its JUCE dependencies propagate transitively:

```cmake
juce_add_module(${PATH_TO}/MosScaleCircleComponent/mos_scale_circle)

target_link_libraries(MyApp PRIVATE mos_scale_circle)
```

Then in code:

```cpp
#include <mos_scale_circle/mos_scale_circle.h>

ScaleStructure scale;                 // the data model
scale.setAll(12, -1, -1);             // 12-note period, suggested generator + size
ScaleStructureComponent editor(scale); // the UI control (ByDegree colours by default)
addAndMakeVisible(editor);
```

## Colour API (two modes)

`ScaleStructure` resolves a degree's colour in one of two modes, selectable by the developer:

- **`ColourMode::ByDegree`** (default) — each degree may have its own colour; degrees with no
  override follow their group's colour.
- **`ColourMode::ByGroup`** — every degree takes the colour of the group it belongs to.

```cpp
scale.setColourMode(ScaleStructure::ColourMode::ByGroup);
scale.setGroupColour(0, juce::Colours::teal);
scale.setDegreeColour(3, juce::Colours::red);   // ByDegree: override; ByGroup: recolours the group
```

`ScaleStructureComponent(scale)` manages its own colour tables. The legacy constructor
`ScaleStructureComponent(scale, juce::Array<juce::Colour>&)` wires a caller-owned table and
selects `ByGroup` (backward-compatible). Hosts that own/persist colours can call
`scale.setGroupColourTable(...)` / `scale.setDegreeOverrideTable(...)` directly.

**Group colours follow their groups.** Entries `[0, groups)` of the group table are the current
groups' colours. Anything past that is set aside, never thrown away.
- Split: the original group keeps its colour, and the new piece takes a set-aside colour.
- Merge: the group that wasn't swallowed keeps its colour, and the swallowed one is set aside.
- Resize: each group keeps the colour of the old group it shares the most degrees with.
- Period, generator, scale size and offset changes: colours stay by position. Fewer groups sets
  the extras aside; more groups brings them back.

A group with no colour yet has a transparent entry. A host that owns the table fills those from
its own palette:

```cpp
scale.fillUnassignedGroupColours([&](int i) { return myDefaultColourFor(i); });
```

This picks the first default that no other group is using, and never shrinks the table. Make
`myDefaultColourFor(i)` return a distinct colour for each `i`, for example by rotating the hue once
the palette runs out. Otherwise two groups can end up with the same colour. Persist
the whole table, set-aside entries included, to keep them across sessions.

**In-wheel picker.** Right-clicking a group or degree assigns a colour. By default a built-in
`juce::ColourSelector` is shown; inject your own picker instead with:

```cpp
circle.setColourPickerLauncher([](juce::Rectangle<int> area, juce::Colour current,
                                  std::function<void(juce::Colour)> onPicked) { /* show picker */ });
```

## Master / follower parameter sync

`ScaleStructure` is backed by a `juce::ValueTree` that is the shared source of truth.

- **Master** — subscribe to changes with granular + catch-all callbacks:

  ```cpp
  struct MyListener : ScaleStructure::Listener {
      void scaleStructurePeriodChanged() override { /* ... */ }
      void scaleStructureOffsetChanged() override { /* ... */ }
      void scaleStructureChanged() override        { /* catch-all */ }
  };
  scale.addListener(&myListener);
  ```

  Or attach a `juce::ValueTree::Listener` to `scale.getState()` for raw binding / persistence /
  undo.

- **Follower** — push parameters in from an external controller; the model and UI update with no
  feedback loop (a no-op write notifies nobody):

  ```cpp
  scale.getState().setProperty(ScaleStructureIDs::period, 19, nullptr);
  // or construct bound to an app-owned tree:
  ScaleStructure bound(appOwnedTree);
  ```

Colours are intentionally **not** part of the parameter tree; they have their own
`groupColourChanged` / `degreeColourChanged` notifications on the wheel.

**Saving a design.** The tree also carries the editing rules (`retainGroupingSymmetry`,
`retainMOSGroupSizes`, `alterationsAttachedToDegree`), so a copy of it is a complete save:

```cpp
juce::ValueTree saved = scale.getState().createCopy();   // persist e.g. via saved.toXmlString()
// ...later, into an existing structure (keeps its own backing tree and listeners):
scale.restoreState(saved);   // false if it isn't a MosScaleStructure with a usable period
```

A grouping or alteration set that no longer fits the restored scale falls back to the suggested
one rather than failing the restore.

**Default offset.** Passing a negative offset to `setAll` (as `resetToPeriod` does) picks
`getSuggestedGeneratorOffset()`: 1 for the fifth-based suggested generator, so the default
scale is Ionian rather than Lydian, and 0 for any other generator.

## Display options

`ScaleStructureComponent` exposes toggles (the last four forward to the wheel):

```cpp
editor.setShowParameters(true);           // sliders/dropdowns/labels (default on; off = wheel only)
editor.setAlwaysShowGroupNumbers(false);  // group-size numbers even when not dragging (default off)
editor.setHighlightOnMouseOver(true);     // recolour the hovered group/degree (default on)
editor.setShowGroupResizeControls(true);  // draggable group-edge handles (default on)
editor.setShowGroups(true);               // outer group ring; off = only the degree ring (default on)
```

## Demo

```sh
cmake -B build -DMOSSC_BUILD_DEMO=ON -DJUCE_PATH=/path/to/JUCE
cmake --build build --target MosScaleCircleDemo
```

The demo (`examples/`) hosts the control with a colour-mode toggle. It also supports headless
checks: `--render <png>` snapshots the wheel, and `--selftest` verifies master/follower behaviour.
