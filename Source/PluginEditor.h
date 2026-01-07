#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Customized UI
class AnnieLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnnieLookAndFeel()
    {
        customTypeface = juce::Typeface::createSystemTypefaceFor (
                                    BinaryData::CalamityJaneNF_ttf,
                                    BinaryData::CalamityJaneNF_ttfSize);
    }
    
    juce::Typeface::Ptr getCustomTypeface() { return customTypeface; }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto isVertical = (style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical);
        float trackWidth = 6.0f;
        auto trackRect = isVertical ? juce::Rectangle<float> (x + width * 0.5f - trackWidth * 0.5f, y, trackWidth, height)
                                    : juce::Rectangle<float> (x, y + height * 0.5f - trackWidth * 0.5f, width, trackWidth);
        g.setColour (juce::Colours::dodgerblue);
        g.fillRoundedRectangle (trackRect.translated (2.0f, 2.0f), trackWidth * 0.5f);
        g.setColour (juce::Colours::hotpink);
        g.fillRoundedRectangle (trackRect, trackWidth * 0.5f);
        auto thumbSize = 18.0f;
        juce::Point<float> thumbPos = isVertical ? juce::Point<float> (x + width * 0.5f, sliderPos)
                                                 : juce::Point<float> (sliderPos, y + height * 0.5f);
        g.setColour (juce::Colours::dodgerblue);
        g.fillEllipse (thumbPos.x - thumbSize * 0.5f + 2.0f, thumbPos.y - thumbSize * 0.5f + 2.0f, thumbSize, thumbSize);
        g.setColour (juce::Colours::hotpink);
        g.fillEllipse (thumbPos.x - thumbSize * 0.5f, thumbPos.y - thumbSize * 0.5f, thumbSize, thumbSize);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 0.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (juce::Colours::dodgerblue);
        g.drawRoundedRectangle (bounds.translated (1.5f, 1.5f), cornerSize, 3.0f);
        g.setColour (juce::Colours::hotpink);
        g.drawRoundedRectangle (bounds, cornerSize, 3.0f);
        
        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) {
            g.setColour (juce::Colours::hotpink.withAlpha (0.1f));
            g.fillRoundedRectangle (bounds, cornerSize);
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        g.setFont (juce::FontOptions (customTypeface).withHeight (20.0f));
        auto textArea = button.getLocalBounds();
        g.setColour (juce::Colours::dodgerblue);
        g.drawFittedText (button.getButtonText(), textArea.translated (1.2f, 1.2f), juce::Justification::centred, 1);
        g.setColour (juce::Colours::hotpink);
        g.drawFittedText (button.getButtonText(), textArea, juce::Justification::centred, 1);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {

        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(25.0f);
        
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        if (slider.getProperties().contains("isWidthKnob"))
        {
            float maxSpreadAngle = juce::MathConstants<float>::pi * 0.75f;
            float currentSpread = sliderPos * maxSpreadAngle;
            
            float shadowOffset = 2.0f;
            float strokeThickness = 6.0f;

            g.setColour (juce::Colours::dodgerblue.withAlpha(0.8f));
            juce::Path bgPath;
            bgPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, -maxSpreadAngle, maxSpreadAngle, true);
            g.strokePath(bgPath, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            if (sliderPos > 0.001f)
            {
                g.setColour (juce::Colours::dodgerblue.withAlpha(0.8f));
                juce::Path shadowPath;
                shadowPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, 0.0f, -currentSpread, true);
                shadowPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, 0.0f, currentSpread, true);
                shadowPath.applyTransform(juce::AffineTransform::translation(shadowOffset, shadowOffset));
                g.strokePath(shadowPath, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                g.setColour (juce::Colours::hotpink);
                juce::Path mainPath;
                mainPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, 0.0f, -currentSpread, true);
                mainPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, 0.0f, currentSpread, true);
                g.strokePath(mainPath, juce::PathStrokeType(strokeThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }

            auto drawThumbWithShadow = [this, &g, centreX, centreY, radius, shadowOffset](float angle) {
                float thumbSize = 16.0f;
                auto dotX = centreX + std::sin (angle) * radius;
                auto dotY = centreY - std::cos (angle) * radius;
                g.setColour (juce::Colours::dodgerblue.withAlpha(0.8f));
                g.fillEllipse (dotX - thumbSize * 0.5f + shadowOffset, dotY - thumbSize * 0.5f + shadowOffset, thumbSize, thumbSize);
                g.setColour (juce::Colours::hotpink);
                g.fillEllipse (dotX - thumbSize * 0.5f, dotY - thumbSize * 0.5f, thumbSize, thumbSize);
            };

            drawThumbWithShadow(-currentSpread);
            drawThumbWithShadow(currentSpread);
        }
        else
        {

            auto radiusOld = (float) juce::jmin (width / 2, height / 2) - 25.0f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour (juce::Colours::dodgerblue);
            juce::Path shadow;
            shadow.addCentredArc (centreX + 2.0f, centreY + 2.0f, radiusOld, radiusOld, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.strokePath (shadow, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour (juce::Colours::hotpink);
            juce::Path main;
            main.addCentredArc (centreX, centreY, radiusOld, radiusOld, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.strokePath (main, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            float dotSize = 16.0f;
            auto dotX = centreX + std::sin (angle) * radiusOld;
            auto dotY = centreY - std::cos (angle) * radiusOld;
            
            g.setColour (juce::Colours::dodgerblue);
            g.fillEllipse (dotX - 6.0f, dotY - 6.0f, dotSize, dotSize);
            g.setColour (juce::Colours::hotpink);
            g.fillEllipse (dotX - 8.0f, dotY - 8.0f, dotSize, dotSize);
        }
    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        const juce::Font font (getLabelFont (label));
        g.setFont (font);
        auto textArea = label.getLocalBounds();
        g.setColour (juce::Colours::dodgerblue);
        g.drawFittedText (label.getText(), textArea.translated (1.5f, 1.5f), juce::Justification::centred, 1);
        g.setColour (juce::Colours::hotpink);
        g.drawFittedText (label.getText(), textArea, juce::Justification::centred, 1);
    }
    
    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::FontOptions (customTypeface).withHeight (26.0f);
    }

private:
    juce::Typeface::Ptr customTypeface;
};

class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AnnieLookAndFeel annieStyle;
    
    juce::Slider aziSlider, eleSlider, widthSlider;
    juce::Label aziLabel, eleLabel, widthLabel;
    juce::TextButton loadHRTFButton { "LOAD HRIR WAV" };
    juce::TextButton clearHRTFButton { "Clear" };
    std::unique_ptr<juce::FileChooser> chooser;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aziAttach, eleAttach, widthAttach;
    
    NewProjectAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
