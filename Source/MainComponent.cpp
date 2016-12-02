#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


#include <limits>

//==============================================================================
/*
This component lives inside our window, and this is where you should put all
your controls and content.
*/
class MainContentComponent : public AudioAppComponent, public Slider::Listener, public TextButton::Listener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize(480, 20+10*30);

        // specify the number of input and output channels that we want to open
        setAudioChannels(2, 2);

        // volume slider
        for (int i = 0; i < 8; i++)
            {
            float value=1./(i+1);
            addAndMakeVisible(volumeSlider_array[i]);
            volumeSlider_array[i].setRange(0, 1);
            volumeSlider_array[i].setTextValueSuffix(" ");
            volumeSlider_array[i].setValue(value);
            volumeSlider_array[i].addListener(this);
            volumeLabel_array[i].setText("osc "+std::to_string(i), dontSendNotification);
            volumeLabel_array[i].attachToComponent(&volumeSlider_array[i], true);
            }

        // freq slider
        addAndMakeVisible(freqSlider);
        freqSlider.setRange(10, 22000);
        freqSlider.setTextValueSuffix(" Hz");
        freqSlider.setValue(500.0);
        freqSlider.addListener(this);
        freqSlider.setSkewFactorFromMidPoint(500);

        freqLabel.setText("Freq", dontSendNotification);
        freqLabel.attachToComponent(&freqSlider, true);
        
        // mute button
        addAndMakeVisible(m_muteButton);
        m_muteButton.setButtonText("Mute");
        m_muteButton.addListener(this);
        m_muteButton.setEnabled(true);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void buttonClicked(Button* button) override
    {
        if (button == &m_muteButton) {
            m_mute = !m_mute;
        }
    }

    void sliderValueChanged(Slider *slider) {
        for (int i = 0; i < 8; i++)
             {
             if (slider == &volumeSlider_array[i]) 
                {
                m_amplitude_array[i] = (float)volumeSlider_array[i].getValue() ;
                }
             }
        

        if (slider == &freqSlider) {
            m_frequency = (float)freqSlider.getValue();
        }

        
    }

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.
        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.
        // For more details, see the help for AudioProcessor::prepareToPlay()

        for (int i = 0; i < 8; i++)
            {
            m_amplitude_array[i] = 1/(i+1);
            }
        m_frequency = 500.0;
        m_time = 0.0;
        m_phase=0;
        m_deltaTime = 1 / sampleRate;
    }

    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        if (m_time >= std::numeric_limits<float>::max()) {
            m_time = 0.0;
        }

        float *monoBuffer = new float[bufferToFill.numSamples];
        
        if (m_mute) return bufferToFill.clearActiveBufferRegion();

        // generate sin wave in mono
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
            float value=0;
            m_phase+=2 * double_Pi * m_frequency*m_deltaTime;
            for (int i = 0; i < 8; i++)
                {
                value = value+m_amplitude_array[i]*sin(i*m_phase);
                }
            monoBuffer[sample] = value;
        }

        // iterate over all available output channels
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            // Get a pointer to the start sample in the buffer for this audio output channel
            float* const buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

            for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
                buffer[sample] = monoBuffer[sample];
            }
        }
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.
        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //==============================================================================
    void paint(Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(Colours::lightgrey);
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        const int sliderLeft = 50;
        freqSlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
        for (int i = 0; i < 8; i++)
            {
            volumeSlider_array[i].setBounds(sliderLeft, 20+(i+1)*30, getWidth() - sliderLeft - 10, 20);
            }

        
        m_muteButton.setBounds(10, 20+9*30, getWidth() - 20, 20);
    }

private:
    //==============================================================================

    // Your private member variables go here...
    float m_amplitude_array[8];
    float m_frequency;
    float m_phase,m_time,m_deltaTime;

    // GUI
    Slider freqSlider;
    Slider volumeSlider_array[8];
    Label volumeLabel_array[8];
    Label volumeLabel,freqLabel;
    TextButton m_muteButton;
    bool m_mute;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
