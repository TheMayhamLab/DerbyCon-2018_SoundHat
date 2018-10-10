This is the derby hat we made for DerbyCon 8.0.

The sound is captured by a mic that transmits the signles to a MSGEQ7 chip. The MSGEQ7 chip splits the sound into seven bands and sends a numaric value to the Arduino. The Arduino takes the value for each band, dose maths, and figures out what LEDs to light up. 

If that's not cool enough, the Arduino also takes a reading of the audio every 2 seconds and levels the volume so it will never be "spiked". (i.e. it will work in most any environment, including concerts.)

I'll try to make a blog post on how it was built as soon as I can, but for now here are the parts you would need.

Parts needed:
- Microphone
- MSGEQ7 graphic EQ display filter (IC)
- Arduino Nano
- 56 LEDS (WS2812B)
- Wire
- Battery

