#include "PluginProcessor.h"
#include "PluginEditor.h"

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&annieStyle);

    auto setupLabel = [this](juce::Label& l, juce::String text) {
        l.setText (text, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, juce::Colours::transparentBlack);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible (l);
    };

    auto setupSlider = [this](juce::Slider& s, juce::String pID, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a, bool rotary) {
        s.setSliderStyle (rotary ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::LinearVertical);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::hotpink);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (s);
        a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), pID, s);
    };

    setupLabel (aziLabel, "Azimuth");
    setupLabel (eleLabel, "Elevation");
    setupLabel (widthLabel, "Width");

    setupSlider (aziSlider, "azimuth", aziAttach, true);
    setupSlider (eleSlider, "elevation", eleAttach, true);
    
    setupSlider (widthSlider, "width", widthAttach, true);
    widthSlider.getProperties().set("isWidthKnob", true);

    addAndMakeVisible (loadHRTFButton);
    loadHRTFButton.onClick = [this] {
        chooser = std::make_unique<juce::FileChooser> ("Select SADIE Folder", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc) {
            auto result = fc.getResult();
            if (result.isDirectory()) {
                audioProcessor.setHRTFDirectory (result);
                repaint();
            }
        });
    };

    addAndMakeVisible (clearHRTFButton);
    clearHRTFButton.onClick = [this] {
        audioProcessor.clearHRTFDirectory();
        repaint();
    };
    
    setSize (460, 400);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor() { setLookAndFeel (nullptr); }

void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF14FFDC)); // Neon Cyan
    
    auto myType = annieStyle.getCustomTypeface();
    if (myType != nullptr)
    {

        g.setFont (juce::FontOptions(myType).withHeight(52.0f));
        g.setColour (juce::Colours::dodgerblue);
        g.drawText ("Annie's 3D Pan", 2, 12, getWidth(), 50, juce::Justification::centred);
        g.setColour (juce::Colours::hotpink);
        g.drawText ("Annie's 3D Pan", 0, 10, getWidth(), 50, juce::Justification::centred);
        
        juce::File path = audioProcessor.getHRTFDirectory();
                juce::String status;
                juce::Colour mainTextColour;
                juce::Colour shadowColour;

                bool isError = false;

                if (!path.exists())
                {
                    status = "Please Select A Folder!";
                    isError = true;
                }
                else if (audioProcessor.getHRTFCacheSize() == 0)
                {
                    status = "ERROR: Invalid Folder!";
                    isError = true;
                }
                else
                {
                    status = "LOADED: " + path.getFileName() + " (" + juce::String(audioProcessor.getHRTFCacheSize()) + " IRs)";
                    isError = false;
                }

                if (isError)
                {
                    mainTextColour = juce::Colours::red;
                    shadowColour = juce::Colour (0xFF2D005F);
                }
                else
                {
                    mainTextColour = juce::Colours::hotpink;
                    shadowColour = juce::Colours::dodgerblue;
                }
        
        
        auto buttonBounds = loadHRTFButton.getBounds();
        
        auto textBounds = juce::Rectangle<int> (buttonBounds.getX(),
                                               buttonBounds.getBottom() + 5,
                                               buttonBounds.getWidth(),
                                               20);

        g.setFont (juce::FontOptions(myType).withHeight(20.0f));
        
        g.setColour (shadowColour);
        g.drawFittedText (status, textBounds.translated(1.2f, 1.2f), juce::Justification::centred, 1);
        
        g.setColour (mainTextColour);
        g.drawFittedText (status, textBounds, juce::Justification::centred, 1);
    }
}

void NewProjectAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    area.removeFromTop (77);
    
    auto footerArea = area.removeFromBottom(98);
    
    footerArea.removeFromTop(17);
    
    auto buttonRow = footerArea.removeFromTop(45).withSizeKeepingCentre(360, 35);
    loadHRTFButton.setBounds (buttonRow.removeFromLeft (260).reduced(5, 0));
    clearHRTFButton.setBounds (buttonRow.removeFromLeft (100).reduced(5, 0));

    auto knobsArea = area.reduced(20, 10);
    int colWidth = knobsArea.getWidth() / 3;

    
    int knobHeight = 200;

    auto col1 = knobsArea.removeFromLeft(colWidth);
    aziLabel.setBounds(col1.removeFromTop(24));
    aziSlider.setBounds(col1.removeFromTop(knobHeight).reduced(0));

    auto col2 = knobsArea.removeFromLeft(colWidth);
    eleLabel.setBounds(col2.removeFromTop(24));
    eleSlider.setBounds(col2.removeFromTop(knobHeight).reduced(0));

    auto col3 = knobsArea;
    widthLabel.setBounds(col3.removeFromTop(24));
    widthSlider.setBounds(col3.removeFromTop(knobHeight).reduced(0));
}
