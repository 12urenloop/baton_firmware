# Steps to get hardware working

- Install "Adafruit nRF52", version 1.3.0 in the Arduino IDE (follow [these instructions](https://learn.adafruit.com/add-boards-arduino-v164/setup))
- Select the board as "Adafruit Feather nRF52832"
- Set the Programmer in Tools to "Bootloader DFU"
- Burn Bootloader (in Tools)
- Change the MAC address in the Arduino sketch
- Upload the firmware

# CAD

In the cad/ folder, there is a 3D model of the cap of the baton

- the FreeCAD source model
- the 3mf export that can be loaded into a slicer
- a gcode file that prints in TPU on the Prusa MK3S; note that the ridges inside of the cap are sliced in 0.1mm layer height
