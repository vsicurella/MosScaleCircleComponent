/*
  ==============================================================================

    Main.cpp
    Minimal JUCE application hosting the MOS Scale Circle demo.

  ==============================================================================
*/

#include <mos_scale_circle/mos_scale_circle.h>
#include "DemoComponent.h"

class MosScaleCircleDemoApplication : public juce::JUCEApplication
{
public:
    MosScaleCircleDemoApplication() = default;

    const juce::String getApplicationName() override       { return "MOS Scale Circle Demo"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String& commandLine) override
    {
        // Offscreen render mode for headless verification: "--render <path.png>".
        auto args = juce::StringArray::fromTokens (commandLine, true);
        int renderIdx = args.indexOf ("--render");
        if (renderIdx >= 0 && renderIdx + 1 < args.size())
        {
            renderToFile (args[renderIdx + 1]);
            quit();
            return;
        }

        int testIdx = args.indexOf ("--selftest");
        if (testIdx >= 0)
        {
            setApplicationReturnValue (runSelfTest());
            quit();
            return;
        }

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    // Headless verification of master/follower behaviour. Returns 0 on success.
    static int runSelfTest()
    {
        bool ok = true;

        // --- MASTER: granular + catch-all notifications fire on changes ---
        struct Probe : public ScaleStructure::Listener
        {
            int period = 0, gen = 0, offset = 0, size = 0, grouping = 0, alter = 0, all = 0;
            void scaleStructurePeriodChanged() override     { ++period; }
            void scaleStructureGeneratorChanged() override  { ++gen; }
            void scaleStructureOffsetChanged() override     { ++offset; }
            void scaleStructureSizeChanged() override       { ++size; }
            void scaleStructureGroupingChanged() override   { ++grouping; }
            void scaleStructureAlterationsChanged() override { ++alter; }
            void scaleStructureChanged() override           { ++all; }
        };

        ScaleStructure scale;
        scale.setAll (12, -1, -1);

        Probe probe;
        scale.addListener (&probe);

        int newOffset = (scale.getGeneratorOffset() + 1) % juce::jmax (1, scale.getScaleSize());
        scale.setGeneratorOffset (newOffset);
        const bool masterOffset = (probe.offset == 1 && probe.all >= 1);
        ok &= masterOffset;

        // --- FOLLOWER: an external tree write drives the model + notifies ---
        auto state = scale.getState();
        const int allBeforePeriod = probe.all;
        state.setProperty (ScaleStructureIDs::period, 19, nullptr);
        const bool follower = (scale.getPeriod() == 19 && probe.period >= 1 && probe.all > allBeforePeriod);
        ok &= follower;

        // --- NO-OP: writing the same value notifies nobody ---
        const int allAfter = probe.all;
        state.setProperty (ScaleStructureIDs::period, 19, nullptr);
        const bool noop = (probe.all == allAfter);
        ok &= noop;

        scale.removeListener (&probe);

        // --- FOLLOWER (UI): external change refreshes the component, captured to disk ---
        ScaleStructure uiScale;
        uiScale.setAll (12, -1, -1);
        ScaleStructureComponent comp (uiScale);
        comp.setSize (760, 800);
        uiScale.getState().setProperty (ScaleStructureIDs::period, 17, nullptr);
        const bool uiFollower = (uiScale.getPeriod() == 17);
        ok &= uiFollower;

        // --- BACK-COMPAT: the legacy Array<Colour>& constructor selects ByGroup ---
        ScaleStructure legacyScale;
        legacyScale.setAll (12, -1, -1);
        juce::Array<juce::Colour> legacyTable;
        for (int i = 0; i < 8; ++i)
            legacyTable.add (juce::Colour::fromHSV ((float) i / 8.0f, 0.6f, 0.9f, 1.0f));
        ScaleStructureComponent legacyComp (legacyScale, legacyTable);
        const bool backCompat = (legacyScale.getColourMode() == ScaleStructure::ColourMode::ByGroup);
        ok &= backCompat;

        auto image = comp.createComponentSnapshot (comp.getLocalBounds());
        juce::File out ("/tmp/mossc_follower.png");
        out.deleteFile();
        if (auto stream = out.createOutputStream())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream (image, *stream);
        }

        juce::Logger::writeToLog (juce::String ("SELFTEST")
            + " master-offset=" + (masterOffset ? "PASS" : "FAIL")
            + " follower=" + (follower ? "PASS" : "FAIL")
            + " noop=" + (noop ? "PASS" : "FAIL")
            + " ui-follower=" + (uiFollower ? "PASS" : "FAIL")
            + " back-compat=" + (backCompat ? "PASS" : "FAIL")
            + "  => " + (ok ? "ALL PASS" : "FAIL"));

        return ok ? 0 : 1;
    }

    static void renderToFile (const juce::String& path)
    {
        DemoComponent demo;
        demo.setSize (900, 720);   // triggers layout of children

        if (path.containsIgnoreCase ("bygroup"))   demo.setColourMode (ScaleStructure::ColourMode::ByGroup);
        if (path.containsIgnoreCase ("groupnums")) demo.getEditor().setAlwaysShowGroupNumbers (true);
        if (path.containsIgnoreCase ("nogroups"))  demo.getEditor().setShowGroups (false);
        if (path.containsIgnoreCase ("noparams"))  demo.getEditor().setShowParameters (false);

        // Render the wheel onto the app's dark background so white hub text is visible.
        auto& editor = demo.getEditor();
        auto bounds = editor.getLocalBounds();
        juce::Image image (juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
        {
            juce::Graphics g (image);
            g.fillAll (juce::Colour (0xff32363f));
            editor.paintEntireComponent (g, false);
        }

        juce::File file (path);
        file.deleteFile();
        if (auto stream = file.createOutputStream())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream (image, *stream);
        }
    }

    void shutdown() override { mainWindow = nullptr; }

    void systemRequestedQuit() override { quit(); }

    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                  .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new DemoComponent(), true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (MosScaleCircleDemoApplication)
