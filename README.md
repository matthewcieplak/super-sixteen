# Super Sixteen Eurorack Sequencecr
Super Sixteen is an open-source/open-hardware sequencer for the Eurorack synthesizer format. It's available through my [web shop](https://store.extralifeinstruments.com) and fine retailers as a complete module and DIY kit. It's developed the Arduino codebase on Atmega328P, using the [PlatformIO](https://platformio.org/) extension for Visual Studio Code.

![Photo of the Super Sixteen module](https://s3.amazonaws.com/extralifeinstruments.com/images/product/supersixteen-metal.jpg)

At just 20HP, it is a compact and powerful 64-step control voltage sequencer with 99 song patch memory, motion recording, scale selection, realtime pitch and rhythm effects, and a uniquely powerful two-handed programming interface.

Video blogs [documenting the sequencer progress are available on youtube](
https://www.youtube.com/playlist?list=PLXcIAQij6ZZKMTHAFv8BHvEnsouNYhx3t).


# Documentation

*   [USER MANUAL (pdf)](tree/master/graphics/manual_1.1.pdf)
*   [BUILD GUIDE (pdf)](http://extralifeinstruments.com/docs/super-sixteen/build-guide.pdf)
*   [BILL OF MATERIALS (octopart list)](https://octopart.com/bom-tool/Wq51xQ3R)
*   INTERACTIVE BOM (html - for easy part placement) [CPU board](http://extralifeinstruments.com/docs/super-sixteen/ibom-cpu.html) / [Control board](http://extralifeinstruments.com/docs/super-sixteen/ibom-control.html)
*   [Video build guide](https://www.youtube.com/watch?v=RGmp3aG8Nbw&feature=emb_title)
*   [1v/oct calibration tutorial video](https://www.youtube.com/watch?v=QJvS-ma6CHY)
*   [Build support thread on Muff Wiggler](https://muffwiggler.com/forum/viewtopic.php?f=97&t=238514)
*   [ModularGrid page](modulargrid.net/e/other-unknown-extralife-instruments-super-sixteen)

# Hardware files
Kicad and gerber files for four PCBs are included in this repository. The project is split into two boards, one for the CPU, memory, power, and DAC logic (CPU_board) and one for the front panel controls (control_board). Each board comes in a through-hole version for DIY builders and an SMT version for automated. They are functionally identical but part numbers are different to reflect the different footprints. 

# Software
Primary development of the firmware takes place in [sequencer_app_v2](tree/master/sequencer_app_v2) using the Arduino C++ libraries but compiled using PlatformIO and typical C header files. (The older, pure Arduino version, [sequencer_app](tree/master/sequencer_app) is here only for archival purposes and is unmaintained). You can find the .hex files to upload using the arduino updater in [sequencer_app_v2/build](tree/master/sequencer_app_v2/build). The shell commands for updating using avrdude are contained in [upload.sh](tree/master/sequencer_app_v2/upload.sh), though you may also use the Arduino IDE to upload new firmware.

# Troubleshooting
Please refer to the [official build support thread on Muff Wiggler](https://muffwiggler.com/forum/viewtopic.php?f=97&t=238514) to post a query. You may find your question has already been answered!

Common issues:
* *My sequencer doesn't turn on!* - Check your soldering carefully, under magnification and bright light! 90% of functional issues can be traced to bad connections caused by missing or cold solder joints. A common place to find bad joints is on the header connector pins, as they are on the opposite side of the board from the othres.
* *My soldering is good, now what?*  After that, check the *orientation* of all your ICs closely. A backwards chip or capacitor or diode is easy to miss.
* *The screen shows "nEn" at startup and freezes when I move the encoder* - This message (actually "mEm") is caused by a communication failure between the CPU (Atmega328p) and the memory chip (W25Q80DV). Most often this is caused by soldering issues on the SMT memory chip. Reflow the SMT joints with flux and try again. If it persists, check for 3.3v at pins 8 and 4 of the memory chip. If that fails, you may suspect the 2n7000 transistors nearby (mosfets are fragile) and then potentially check the integrity of the traces between the chip and the resistors. Opening kicad and using the show net tool is helpful for this.
* *I can program sequences but they're not in tune* You need to perform the DAC calibration procedure, as outlined in the manual and [this video tutorial](https://www.youtube.com/watch?v=QJvS-ma6CHY). You'll need a multimeter or electronic tuner.


# License
Code: [GPL3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)
Hardware: [cc-by-sa-3.0](https://creativecommons.org/licenses/by-sa/3.0/deed.en)

By: Matthew Cieplak (matthew.cieplak@gmail.com)
