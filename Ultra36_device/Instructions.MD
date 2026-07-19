# Ultra-36 Device Instructions

Ultra-36 is an internal ROM selector for the Commodore 128. The assembled board plugs into the computer's U36 function-ROM socket and uses an ATtiny84A to select banks in an SST39SF020A or SST39SF040 flash device.

Read the complete guide before soldering or installing the board. Component values, jumper positions, and connector labels must agree with the revision of the PCB being assembled.

## Safety

- Disconnect the Commodore 128 from power and all peripherals before opening it.
- Use antistatic precautions when handling the computer, ATtiny84A, and flash device.
- Check every socket and IC orientation before applying power. Reversed parts can damage the board or the computer.
- Inspect the completed assembly for solder bridges and short circuits before inserting it into U36.
- Installation inside the computer should be performed only by someone comfortable working on vintage electronics.

## Required parts and tools

- Ultra-36 PCB
- ATtiny84A in a DIP-14 package
- SST39SF020A for 8 banks or SST39SF040 for 16 banks
- Resistor and capacitor specified for R1 and C1 on the PCB revision
- Machine-pin strips and IC sockets matching the PCB footprints
- Header pins and a jumper shunt(just double 2.54 header pin) for JP1
- Temperature-controlled soldering iron, solder, side cutters, and a multimeter
- ISP programmer compatible with the ATtiny84A
- Flash programmer compatible with the selected SST39SF device

## Assembly

### 1. Inspect and orient the PCB

Locate the component outlines, reference labels, pin 1 marks, and the U36 connector footprint. The printed side shown below is the component side. Do not rely on the photograph alone for component values; follow the markings and documentation supplied with the PCB revision.

![Bare Ultra-36 PCB](Ultra36_board_assembly_images/Ultra36_1.png)

### 2. Fit the low-profile components

Install R1 and C1 first, followed by the ATtiny84A or its socket as required by the PCB revision. If the ATtiny84A is soldered directly, align its pin 1 indicator with the PCB marking. Keep each component seated close to the board and inspect the joints after soldering.

![ATtiny84A, resistor, and capacitor fitted](Ultra36_board_assembly_images/Ultra36_2.png)

### 3. Align the connector strips

Fit the machine-pin strips that will plug into the C128 U36 socket. A solderless breadboard or spare socket can hold the strips straight and parallel while the first pins are soldered. Tack one pin at each end, confirm the alignment, and then solder the remaining pins.

![Connector strips aligned before soldering](Ultra36_board_assembly_images/Ultra36_3.png)

### 4. Install the flash socket and headers

Install the socket for the SST39SF flash device. Match the socket notch to the pin 1 marking on the PCB. Fit the remaining labeled headers and the JP1 jumper header, keeping them square to the board.

![Flash socket and headers during assembly](Ultra36_board_assembly_images/Ultra36_4.png)

### 5. Inspect the completed board

Before inserting either programmable device:

1. Check the underside for missed joints, solder bridges, and clipped leads touching adjacent pads.
2. Verify continuity from each U36 connector pin to its corresponding socket pad.
3. Verify that power and ground are not shorted.
4. Confirm the orientation of both IC sockets and all headers.

![Completed Ultra-36 controller board](Ultra36_board_assembly_images/Ultra36_5.png)

## Programming

### ATtiny84A firmware

Download the companion ATtiny84A firmware from [lukassoft.com/ultra36](https://lukassoft.com/ultra36). The firmware expects an 8 MHz internal clock and no bootloader. Set the required fuses using the programming instructions supplied with the download, then write the firmware and verify it before placing the controller on the board.

### Function ROM image

Use a complete flash image tool online [lukassoft.com/ultra36](https://lukassoft.com/ultra36) or create manually following the [`Ultra36MenuRom`](../README.md) project:

- SST39SF020A: 256 KB, providing 8 banks of 32 KB
- SST39SF040: 512 KB, providing 16 banks of 32 KB
- Bank 0 contains the Ultra-36 menu.
- Bank 1 must contain the 32 KB `Empty_Bank` image.
- Any 16 KB ROM must be padded to 32 KB before it is included in the combined image.(Online tool does it automatically)

Write the combined image to the flash device with a compatible programmer and perform a verify pass after programming.

![Ultra-36 board with compatible SST39SF flash devices](Ultra36_board_assembly_images/Ultra36_6.png)

## Jumper and signal connections

JP1 jumper provides power to SST flash chip, disconnect when flashing ATTiny84A or though removing flash is recommended.

The menu communicates with the ATtiny84A through two C128 User Port signals:

| C128 signal | ATtiny84A signal | Purpose |
| --- | --- | --- |
| User Port C / CIA2 PB0 | PA1 | Serial data |
| User Port D / CIA2 PB1 | PB2 / INT0 | Serial clock |

Connect these signals, the reset monitor, and the optional JiffyDOS control only to the matching labeled board points. The Ultra-36 board receives power and ground through the U36 socket. Optional 330 ohm to 1 kohm series resistors may be used on the data and clock wires.

## Installation in the Commodore 128

1. Disconnect the computer from power and remove its case and shielding as required.
2. Locate the U36 function-ROM socket and note its pin 1 orientation before removing the existing device, if fitted.
3. Insert the programmed ATtiny84A and flash device into their sockets, matching every pin 1 indicator to the PCB markings.
4. Align the Ultra-36 connector with U36. Confirm that every pin enters the socket and that no row is offset.
5. Route the signal wires so they cannot be pinched by the shielding or case.
6. Recheck orientation, jumper position, power-to-ground resistance, and cable routing before reconnecting power.

## First power-on and test

On first power-up, bank 0 should display the Ultra-36 menu. Select a known ROM and restart the computer to confirm bank switching and persistent selection. Holding RESET for approximately three seconds should return the selector to menu bank 0.

If the computer does not start, disconnect power immediately. Recheck U36 alignment, IC orientation, JP1, firmware fuse settings, flash contents, solder joints, and the User Port data and clock connections before trying again.

Rremember you are modifiying your computer at your own risk!!!