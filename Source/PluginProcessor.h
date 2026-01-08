#pragma once
#include <JuceHeader.h>

class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:

    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setHRTFDirectory (const juce::File& newDir);
    juce::File getHRTFDirectory() const { return hrtfRoot; }
    
    int getHRTFCacheSize() const { return (int)hrtfCache.size(); }
    
    void clearHRTFDirectory();

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
   
    struct HRTFRecord
    {
        float azimuth;
        float elevation;
        //juce::File file;
       
        juce::AudioBuffer<float> irData;
        double sampleRate;
    };
    
    juce::File hrtfRoot;
    std::vector<HRTFRecord> hrtfCache;
    double currentSampleRate = 44100.0;
    
    

    float lastAziL = -1000.0f, lastEleL = -1000.0f;
    float lastAziR = -1000.0f, lastEleR = -1000.0f;

    
    juce::AudioBuffer<float> spatialLBuffer;
    juce::AudioBuffer<float> spatialRBuffer;

    void loadHRTFDatabaseToMemory (double sampleRate);
    void updateKernels (float aziL, float eleL, float aziR, float eleR);
    const HRTFRecord* findBestMatch (float azi, float ele) const;
    
    //smooth parameters
    juce::LinearSmoothedValue<float> smoothedAzi;
    juce::LinearSmoothedValue<float> smoothedEle;
    juce::LinearSmoothedValue<float> smoothedWidth;
    
    //set latency
    //I tried zero latency, but when I listened to it in Ableton, I still felt there was phase cancellation.  The minimum latency I could set in JUCE convolution was 512 samples, so I set them to a 512-sample delay.
    //But the thing is it create a zipper noise when rotate the knob
//    juce::dsp::Convolution convL { juce::dsp::Convolution::Latency { 512 } };
//    juce::dsp::Convolution convR { juce::dsp::Convolution::Latency { 512 } };
    
    juce::dsp::Convolution convL, convR;
    //But there is still zipper noise....

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
