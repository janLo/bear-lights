# Fancy costume lights

This is a hacked-together-solution for fancy costume-lighting.

Please don't expect a high-quality project, as the main goal was to finish this before an event ;).

It supports color-changing to detected beats and several other programms for the TM1829.

It can drive single-mode-rgb-leds and TM1829 strips.
The code for the TM1829 was taken from [chriszero/neobob](https://github.com/chriszero/neobob).
The beat detection is from [Beat Detection On The Arduino by DAMIAN PECKETT](http://damian.pecke.tt/beat-detection-on-the-arduino).

To build this you need the schematic from [Beat Detection On The Arduino](http://damian.pecke.tt/beat-detection-on-the-arduino).
The input that the progeam uses is still A0.
On A1 should be a 10K potentiometer to select the beat threshold.
The switch to select the program is on D3.
The brigntness can be selected by a button on D4.
Both switches need a pull-down-resistor.

You can connect a simple LED on D2 to see the beat-detection in action.

The Pins D10-D12 are used for single-color-strips RGB PWM signals.
You can connect them to a simple driver and run your strips with it.

D13 is the data-pin dor the TM1829 strips.

The whole code is licensed under GPLv3.
