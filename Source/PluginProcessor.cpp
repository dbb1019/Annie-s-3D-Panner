#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
    static inline float wrap360 (float a) {
        while (a < 0.0f) a += 360.0f;
        while (a >= 360.0f) a -= 360.0f;
        return a;
    }
    
    static inline float circularDistanceDeg (float a, float b) {
        float d = std::abs (a - b);
        return std::min (d, 360.0f - d);
    }
}

NewProjectAudioProcessor::NewProjectAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{}

NewProjectAudioProcessor::~NewProjectAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"azimuth", 1},   "Azimuth",      0.0f, 360.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"elevation", 1}, "Elevation",   -90.0f, 90.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"width", 1},     "Width",        0.0f, 100.0f, 0.0f));
    
    return layout;
}

void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 2;

    convL.prepare (spec);
    convR.prepare (spec);

    spatialLBuffer.setSize (2, samplesPerBlock);
    spatialRBuffer.setSize (2, samplesPerBlock);
    
    smoothedAzi.reset (sampleRate, 0.1);
    smoothedEle.reset (sampleRate, 0.1);
    smoothedWidth.reset (sampleRate, 0.1);
    
    smoothedAzi.setCurrentAndTargetValue (apvts.getRawParameterValue ("azimuth")->load());
    smoothedEle.setCurrentAndTargetValue (apvts.getRawParameterValue ("elevation")->load());
    smoothedWidth.setCurrentAndTargetValue (apvts.getRawParameterValue ("width")->load());
    
    loadHRTFDatabaseToMemory (sampleRate);
    
    //report latency to daw to fix phase issue
    //setLatencySamples(convL.getLatency());
    
    int currentLatency = convL.getLatency();
        setLatencySamples (currentLatency);
    
    //check latency
    DBG("LATENCY CHECK: " << currentLatency);
}

void NewProjectAudioProcessor::setHRTFDirectory (const juce::File& newDir)
{
    hrtfRoot = newDir;
    loadHRTFDatabaseToMemory (getSampleRate());
}

void NewProjectAudioProcessor::loadHRTFDatabaseToMemory (double sampleRate)
{
    //clear previous hrtf data
    hrtfCache.clear();
    
    if (!hrtfRoot.isDirectory())
        return;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    // select foder based on sample rate
    juce::String folderName = (sampleRate <= 44100.0) ? "44K_16bit" : (sampleRate <= 48000.0 ? "48K_24bit" : "96K_24bit");
    juce::File targetDir = hrtfRoot.getChildFile (folderName);

    if (!targetDir.exists())
    {
        DBG("Target HRTF subdirectory not found: " + targetDir.getFullPathName());
        return;
    }

    auto files = targetDir.findChildFiles (juce::File::findFiles, false, "*.wav");
    
    if (files.isEmpty()) {
            DBG("Error: No .wav files found in: " + targetDir.getFullPathName());
            return;
        }
    
    // Iterate through all files and load them into memory.
    for (auto& file : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        
        if (reader != nullptr)
        {
            HRTFRecord record;
            auto name = file.getFileNameWithoutExtension();
            

            record.azimuth = wrap360(name.fromFirstOccurrenceOf ("azi_", false, false).upToFirstOccurrenceOf ("_ele", false, false).getFloatValue());
            record.elevation = juce::jlimit(-90.0f, 90.0f, name.fromFirstOccurrenceOf ("ele_", false, false).getFloatValue());
            
            // record.irData
            record.sampleRate = reader->sampleRate;
            record.irData.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
            reader->read (&record.irData, 0, (int) reader->lengthInSamples, 0, true, true);
            
            hrtfCache.push_back (std::move (record));
        }
    }
    
    DBG("Successfully cached " + juce::String(hrtfCache.size()) + " HRTF files into RAM.");
    
    if (currentSampleRate > 0)
        setLatencySamples(convL.getLatency());
}

const NewProjectAudioProcessor::HRTFRecord* NewProjectAudioProcessor::findBestMatch (float azi, float ele) const
{
    if (hrtfCache.empty()) return nullptr;

    const HRTFRecord* best = nullptr;
    float minD = std::numeric_limits<float>::max();

    for (auto& item : hrtfCache)
    {
        float dA = circularDistanceDeg (item.azimuth, azi);
        float dE = std::abs (item.elevation - ele);
        float d = dA + dE;
        if (d < minD) { minD = d; best = &item; }
    }
    return best;
}

void NewProjectAudioProcessor::updateKernels (float aziL, float eleL, float aziR, float eleR)
{
    
    auto loadFromMemory = [this](juce::dsp::Convolution& conv, float azi, float ele, float& lastAzi, float& lastEle)
    {
      
        if (std::abs(azi - lastAzi) > 0.1f || std::abs(ele - lastEle) > 0.1f)
        {
            if (auto* match = findBestMatch(azi, ele))
            {
                juce::AudioBuffer<float> irCopy;
                irCopy.makeCopyOf(match->irData);
                
                conv.loadImpulseResponse (std::move(irCopy),
                                          match->sampleRate,
                                          juce::dsp::Convolution::Stereo::yes,
                                          juce::dsp::Convolution::Trim::yes,
                                          juce::dsp::Convolution::Normalise::yes);
                
                lastAzi = azi;
                lastEle = ele;
            }
        }
    };

    loadFromMemory (convL, aziL, eleL, lastAziL, lastEleL);
    loadFromMemory (convR, aziR, eleR, lastAziR, lastEleR);
}

void NewProjectAudioProcessor::clearHRTFDirectory()
{
    hrtfRoot = juce::File();
    hrtfCache.clear();
    
    convL.reset();
    convR.reset();
    
    lastAziL = -1000.0f; lastEleL = -1000.0f;
    lastAziR = -1000.0f; lastEleR = -1000.0f;
    
    DBG("HRTF Path and Cache Cleared.");
}

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();


    smoothedAzi.setTargetValue (apvts.getRawParameterValue ("azimuth")->load());
    smoothedEle.setTargetValue (apvts.getRawParameterValue ("elevation")->load());
    smoothedWidth.setTargetValue (apvts.getRawParameterValue ("width")->load());


    
    if (hrtfCache.size() > 0) //if there is a folder selected and HRIR is loaded - 3d pan mode
    {
        const float azi   = smoothedAzi.getNextValue();
        const float ele   = smoothedEle.getNextValue();
        const float width = smoothedWidth.getNextValue();

        smoothedAzi.skip(numSamples);
        smoothedEle.skip(numSamples);
        smoothedWidth.skip(numSamples);
      
        const float widthOffset = (width / 100.0f) * 90.0f;
        float aziR = wrap360 (azi - widthOffset);
        float aziL = wrap360 (azi + widthOffset);
        
        
        updateKernels (aziL, ele, aziR, ele);
        
        spatialLBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
        spatialLBuffer.copyFrom (1, 0, buffer, 0, 0, numSamples);
        
        int srcR = buffer.getNumChannels() > 1 ? 1 : 0;
        spatialRBuffer.copyFrom (0, 0, buffer, srcR, 0, numSamples);
        spatialRBuffer.copyFrom (1, 0, buffer, srcR, 0, numSamples);
        
        juce::dsp::AudioBlock<float> blockL (spatialLBuffer), blockR (spatialRBuffer);
        convL.process (juce::dsp::ProcessContextReplacing<float>(blockL));
        convR.process (juce::dsp::ProcessContextReplacing<float>(blockR));
        
        
        buffer.setSize (2, numSamples, false, false, true);
        auto* outL = buffer.getWritePointer(0);
        auto* outR = buffer.getWritePointer(1);
        
        
        // Make-Up Gain
        // 4.0f (+12dB)
        const float makeUpGain = 4.0f;
        
        for (int s = 0; s < numSamples; ++s)
        {
            // 0.707f is used to prevent clipping due to the superposition of dual mono signals, and makeUpGain is used to compensate for convolution losses.
            outL[s] = (spatialLBuffer.getReadPointer(0)[s] + spatialRBuffer.getReadPointer(0)[s]) * 0.707f * makeUpGain;
            outR[s] = (spatialLBuffer.getReadPointer(1)[s] + spatialRBuffer.getReadPointer(1)[s]) * 0.707f * makeUpGain;
        }
        
    }
    
    else //if no HRIR data is loaded - stereo pan
        {
            auto calculateGains = [](float pos, float& leftGain, float& rightGain) {
                float norm = (pos + 1.0f) * 0.5f;
                float angle = norm * juce::MathConstants<float>::halfPi;
                leftGain = std::cos(angle);
                rightGain = std::sin(angle);
            };

            auto* inL = buffer.getReadPointer(0);
            auto* inR = numChannels > 1 ? buffer.getReadPointer(1) : inL;
            auto* outL = buffer.getWritePointer(0);
            auto* outR = buffer.getWritePointer(1);

            for (int s = 0; s < numSamples; ++s)
            {
                const float currentAzi = smoothedAzi.getNextValue();
                const float currentWidth = smoothedWidth.getNextValue();
                smoothedEle.getNextValue();

                float pan = -std::sin (juce::degreesToRadians (currentAzi));
                
                float widthNorm = currentWidth / 100.0f;

                float posL = juce::jlimit(-1.0f, 1.0f, pan - widthNorm);
                float posR = juce::jlimit(-1.0f, 1.0f, pan + widthNorm);

                float tLL, tLR, tRL, tRR;
                calculateGains(posL, tLL, tLR);
                calculateGains(posR, tRL, tRR);
                
                //compensate -4.5db
                float panAbs = std::abs(pan);
                float curve = std::cos(panAbs * juce::MathConstants<float>::halfPi);
                float compensation = 1.0f - (1.0f - 0.596f) * curve;

                float finalLL = tLL * compensation;
                float finalLR = tLR * compensation;
                float finalRL = tRL * compensation;
                float finalRR = tRR * compensation;

                float L = inL[s];
                float R = inR[s];
                
                outL[s] = (L * finalLL) + (R * finalRL);
                outR[s] = (L * finalLR) + (R * finalRR);
            }
        }
}

void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("hrtfPath", hrtfRoot.getFullPathName(), nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        
        juce::String savedPath = apvts.state.getProperty("hrtfPath", "");
        if (savedPath.isNotEmpty())
        {
            hrtfRoot = juce::File(savedPath);
            loadHRTFDatabaseToMemory(getSampleRate());
        }
    }
}

void NewProjectAudioProcessor::releaseResources() {}

bool NewProjectAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
