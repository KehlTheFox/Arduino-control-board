**An Arduino sketch controlling a custom board.**

***
**Basic Idea**: Initially, there was a need to control a variety of devices, with the ability to change their operating algorithms (on/off logic, timers) without directly redesigning the entire circuit.

The Arduino platform is excellent for implementing digital logic and can both receive logic signals from switches and toggle switches and send control signals to actuators. 
The software environment allows for quick and easy creation, editing, and updating of large and complex sketches.
However, the number of GPIOs on standard Arduino boards is limited.

Therefore, a very simple, modular board was created to increase the number of input and output pins on the Arduino.
Shift registers (74HC165 and 74HC595) were chosen as a method for expanding the number of GPIOs. Using 4 (and 3) pins, the shift registers add 8 GPIOs each and can be scaled to 8, 16, 24, or 128 GPIOs.

***
**Board implementation**: The board and sketch are built according to the "Receive - Analyze - Process - Send" logic.

The prototype board uses 2 input registers (16 GPIs) and 3 output registers (24 GPOs). The registers can be either permanently soldered to the board or removable, using special connectors for easy installation and removal.
An Arduino UNO was used as the processing center - this applies to both original and aftermarket UNO boards.
The 74HC165 receives input signals. The Arduino then processes them and sends control signals using the 74HC595.

In this case, this custom board replaces the original light control unit and the "turn signal and hazard warning relay" in a Nissan Laurel, controlling the relay blocks that already control various lamps and other devices. 
Transistors, MOSFETs, and anything else that can be controlled by logical "1" and "0" can be used instead of relay blocks.

***
**Sketch implementation**: Almost every line of code is commented within the sketch. I believe this is sufficient to ensure the code is understandable and transparent.

The "Receive-Analyze-Process-Send" principle is also used in the code, creating a strict program flow.

First, data is read from the input register and stored in a variable.
Then, based on this, the program logic decides what needs to be turned on, off, and so on.
Each block of code writes which devices will be turned on and which off to another variable.
Finally, this information is sent to the output register.

Then the cycle repeats again and again.
