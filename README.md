# Annie's 3D Pan

It moves sound left, it moves sound right... 

It moves sound up, it moves sound down... 

It moves sound forward, it moves sound behind...
 

࣪ ˖ ࣪ ᨰꫀᥣᥴ᥆ꩇꫀ ! ᰔ ִ°🥂⋆.ೃ🍾࿔*:･ ׄ

<img src="Annie's%203D%20Panner.jpg" alt="Annie's Panner Screenshot" width="100%">

Annie's Panner is a binaural 3D panner made with **JUCE**.  

It features a dual-engine design that automatically switches between a Binaural 3D Panner (using HRTF convolution) and a standard Stereo Panner, depending on whether HRTF data is loaded.


## Demo Video

[Download the demo video here](https://drive.google.com/file/d/1oVSCJ9u0aT9XKh8qeN_VLBqWG6-DZC0T/view?usp=sharing): https://drive.google.com/file/d/1oVSCJ9u0aT9XKh8qeN_VLBqWG6-DZC0T/view?usp=sharing

(Note: Please wear headphones 🎧 for the binaural effect)

## Modes

### Binaural 3D Mode
#### Trigger: Active when a valid HRTF folder is loaded.

- This mode uses real-time convolution to simulate 3D space over headphones. By convolving audio with Head-Related Impulse Responses (HRIR), it tricks the brain into perceiving sound sources from specific directions. The plugin splits the stereo signal into two distinct sources in the virtual 3D space.

- Azimuth: Rotates the sound 360° around the head. (0° - front; 90° - left;  180° - behind; 270° - right)

- Elevation: Moves the sound vertically (-90° down to +90° up).

- Width: Controls the separation of the left/right input channels in the 3D space.

- The plugin detects the DAW's sample rate and loads the corresponding HRIRs (e.g., searching for 44K_16bit, 48K_24bit, or 96K_24bit subfolders).

- Latency Compensation: FFT cause latency, this plugin reports latency to the DAW.

### Stereo Pan Mode
#### Trigger: Active when no HRTF folder is loaded (or after clicking "Clear").

- If no HRTF data is present, the plugin functions as a stereo panner.

- Azimuth: Acts as a standard pan knob. (0° - middle; 90° - left;  180° - middle; 270° - right)

- Width: Adjusts the stereo width.

## HRTF Setup Guide
To use the 3D features, you need to load Head-Related Impulse Responses (HRTF). This plugin uses the SADIE II HRTF Database developed by the University of York.

I have included 6 different HRTF models in the SADIE folder of this repository to get you started immediately:

- D1, D2 (Dummy Head Measurements)

- H3, H4, H12, H20 (Human Subject Measurements)

#### How to Load:
- Click the "LOAD HRIR WAV" button in the plugin.

- Navigate to the SADIE folder included in this download.

- Select one of the subject folders (e.g., D1_HRIR_WAV or H20_HRIR_WAV) and click Open.

#### Want more models?
If you want to experiment with different head shapes and ear characteristics, you can download the full database from the official website: https://www.york.ac.uk/sadie-project/database.html

---

### Installation

#### Option 1: Download Binaries
[Download macOS here](https://github.com/dbb1019/Annie-s-3D-Panner/releases/tag/v0.9.0)

Move the files to:

- VST3 ```~/Library/Audio/Plug-Ins/VST3/```

- AU (.component) ```~/Library/Audio/Plug-Ins/Components/```

macOS Security Fix: If blocked, go to System Settings > Privacy & Security and click "Open Anyway".
(Or run ```sudo xattr -cr [plugin_path]``` in Terminal).

#### Option 2: Build from Source

- Clone the repo: ```git clone https://github.com/dbb1019/Annie-s-3D-Panner.git```

- Open ```Annie's 3D Panner.jucer``` in Projucer.

- Export to Xcode or Visual Studio and build.
