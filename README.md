# GloweyCore
Firmware for the Glowey Cores made by Loial

![Small Glowey Core](docs/Small%20glowey%20core%20overall.jpg)
![Small Glowey Core](docs/Small%20glowey%20core.jpg)
![Larger Glowey Core](docs/Larger%20glowey%20core.jpg)

-----------------------------

Programming
-----------

You can program this into a glowey core using a specially made cable. It requires a standard USB cable with SWD JTAG pins on the data pair.

You must provide at least 3V and at most 5V on the power pins for the core to turn on JTAG.

Also important for initial programming is applying the option bytes so the core will boot. This is due to the BOOT pin being re-used as the
interrupt pin from the accelerometer. This pin is generally held high - the option bytes change the boot function to boot from normal code
instead of the standard bootloader embedded in the CPU.


Schematic
---------

[Schematic](docs/LED%20Crystals%20V4.0.pdf)


