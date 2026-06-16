/*
  ==============================================================================

    ScaleStructureIDs.h

    juce::Identifiers for the primary (input) parameters of a ScaleStructure.
    These name the properties stored in the ScaleStructure's backing ValueTree
    (the single source of truth). Derived data (generator chain, groupings,
    scale sizes, colours) is computed and is NOT stored in the tree.

  ==============================================================================
*/

#pragma once

namespace ScaleStructureIDs
{
    // Root tree type
    static const juce::Identifier MosScaleStructure ("MosScaleStructure");

    // Primary parameters
    static const juce::Identifier period            ("period");
    static const juce::Identifier generatorIndex    ("generatorIndex");
    static const juce::Identifier sizeIndex         ("sizeIndex");
    static const juce::Identifier generatorOffset   ("generatorOffset");
    static const juce::Identifier periodFactorIndex ("periodFactorIndex");

    // Degree grouping size-indices, stored as a comma-separated string of ints
    static const juce::Identifier degreeGroups      ("degreeGroups");

    // Chroma alterations, stored as a comma-separated string of "x:y" points
    static const juce::Identifier chromaAlterations ("chromaAlterations");
}
