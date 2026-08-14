# Ultra-36 Device Instructions

Ultra-36 is an internal ROM selector for the Commodore 128. The assembled board plugs into the computer's U36 function-ROM socket and uses an ATtiny84A to select banks in an SST39SF020A or SST39SF040 flash device.

Read this complete guide before soldering or installing the board. Component values, jumper positions, and connector labels must match the revision of the PCB being assembled.

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
- 32-pin DIP precision socket
- R1: 10 kΩ resistor and C1: 0.1 µF capacitor, in mini sizes
- Machine-pin strips and IC sockets matching the PCB footprints; precision gold-pin sockets are recommended
- Header pins, straight or angled, and a jumper shunt for JP1 (2-pin, 2.54 mm pitch)
- Temperature-controlled soldering iron, solder, side cutters, and a multimeter
- ISP programmer compatible with the ATtiny84A
- Flash programmer compatible with the selected SST39SF device

### User Port adapter

- Ultra-36 User Port adapter PCB
- 24-pin 805 flange card-edge connector, 3.96 mm pitch
- Two 470 Ω resistors and one 1 kΩ resistor
- 2.54 mm header pins
- Chisel-tip soldering iron; a fine-point tip may also be useful
- An optional momentary switch is not recommended: the C128 User Port reset signal resets only the IEC bus, not the computer, so it is not suitable for controlling Ultra-36.

## Assembly

### 1. Inspect and orient the PCB

Locate the component outlines, reference labels, pin-1 marks, and the U36 connector footprint. The printed side shown below is the component side. Do not rely on a photograph for component values; follow the markings and documentation supplied with the PCB revision.

![Bare Ultra-36 PCB](Ultra36_board_assembly_images/Ultra36_1.jpg)

### 2. Fit the low-profile components

Install R1 and C1 first, followed by the ATtiny84A socket if your PCB revision uses one. If the ATtiny84A is soldered directly, align its pin-1 indicator with the PCB marking. The programming header allows the microcontroller to be reprogrammed after assembly. Keep each component seated close to the board and inspect every joint after soldering.

![ATtiny84A socket area with R1 and C1 fitted](Ultra36_board_assembly_images/Ultra36_2.jpg)

### 3. Align the connector strips

Fit the machine-pin strips that will plug into the C128 U36 socket. A solderless breadboard or spare socket can hold the strips straight and parallel while the first pins are soldered. Tack one pin at each end, confirm the alignment, and then solder the remaining pins.

![Machine-pin connector strips aligned in a breadboard](Ultra36_board_assembly_images/Ultra36_3.jpg)

### 4. Install the flash socket and headers

Install the socket for the SST39SF flash device. Match the socket notch to the pin-1 marking on the PCB. Fit the remaining labeled headers and the JP1 jumper header, keeping them square to the board.

![Flash socket and headers being fitted](Ultra36_board_assembly_images/Ultra36_4.jpg)

### 5. Inspect the completed board

Before inserting the programmable devices:

1. Check the underside for missed joints, solder bridges, and clipped leads touching adjacent pads.
2. Verify continuity from each U36 connector pin to its corresponding socket pad.
3. Verify that power and ground are not shorted.
4. Confirm the orientation of both IC sockets and all headers.

![Assembled Ultra-36 controller board](Ultra36_board_assembly_images/Ultra36_5.jpg)

### 6. Inspect the User Port adapter PCB

Confirm the connector, resistor, and header footprints before soldering. The PCB markings identify R1 and R2 as 470 Ω and R3 as 1 kΩ.

![Bare Ultra-36 User Port adapter PCB](Ultra36_board_assembly_images/UltraAdapter_1.jpg)

### 7. Prepare and solder the edge connector

Apply a small amount of flux to the edge-connector contacts.
Apply solder on all contacts before next step.
 Bend the connector contacts slightly inward, slide the PCB between them, and align the connector with the board edge. Inspect both sides before soldering. Use a chisel-tip iron and enough solder to wet each contact fully; do not leave contacts bridged together.

![User Port adapter PCB prepared for edge-connector installation](Ultra36_board_assembly_images/UltraAdapter_2.jpg)

![Edge connector contacts being soldered](Ultra36_board_assembly_images/UltraAdapter_3.jpg)

![Soldered edge connector on the adapter PCB](Ultra36_board_assembly_images/UltraAdapter_4.jpg)

### 8. Install the resistors and header pins

1. Install R1 and R2 as 470 Ω resistors and R3 as a 1 kΩ resistor. R1 and R2 may be replaced with any value from 330 Ω to 1 kΩ 
2. Install the header pins.
3. Clean flux residue from the PCB with isopropyl alcohol or PCB cleaner.
4. Verify continuity for all three connectors.

![Completed User Port adapter with resistors and headers](Ultra36_board_assembly_images/UltraAdapter_5.jpg)

### 9. Make the connection cable

1. Prepare three 30 cm jumper wires.
2. At the User Port end, use one pin for reset and a 2-pin connector for communication: data and clock.
3. At the Ultra-36 end, use a 4-pin connector. Make sure connectors are faced up so you see metal elements.
4. The Ultra-36 pins are numbered 0–3.
5. Inspect every connector and wire for secure joints and correct pin order.
6. Ultra-36 pin 1 (second from left) is the JiffyDOS-enable pin. Do not connect the JiffyDOS ground wire. Connect only the wire from the top of the JiffyDOS EPROMs; Ultra-36 provides the ground connection.

![User Port adapter and connection cable](Ultra36_board_assembly_images/UltraWire_1.jpg)

![Connection cable attached to the User Port adapter and Ultra-36 board](Ultra36_board_assembly_images/UltraWire_2.jpg)

![Ultra-36 installed on a C128 motherboard](Ultra36_board_assembly_images/UltraComplete_1.jpg)

![Completed Ultra-36 installation inside a C128](Ultra36_board_assembly_images/UltraComplete_2.jpg)

## Programming

### ATtiny84A firmware

Download the companion ATtiny84A firmware from [lukassoft.com/ultra36](https://lukassoft.com/ultra36). The firmware expects an 8 MHz internal clock and no bootloader. Set the required fuses using the programming instructions supplied with the download, then write the firmware and verify it before installing the controller on the board.

### Function ROM image

Use the complete flash-image tool at [lukassoft.com/ultra36](https://lukassoft.com/ultra36), or create the image manually by following the [`Ultra36MenuRom`](../README.md) project instructions:

- SST39SF020A: 256 KB, providing 8 banks of 32 KB
- SST39SF040: 512 KB, providing 16 banks of 32 KB
- Bank 0 contains the Ultra-36 menu.
- Bank 1 must contain the 32 KB `Empty_Bank` image.
- Pad every 16 KB ROM to 32 KB before including it in the combined image. The online tool does this automatically.

Write the combined image to the flash device with a compatible programmer and perform a verify pass after programming.

![Ultra-36 board with compatible SST39SF flash devices](Ultra36_board_assembly_images/Ultra36_6.jpg)

## Jumper and signal connections

The JP1 jumper provides power to the SST flash chip. Disconnect JP1 when programming the ATtiny84A; removing the flash device is recommended.

The menu communicates with the ATtiny84A through two C128 User Port signals:

| C128 signal | ATtiny84A signal | Purpose |
| --- | --- | --- |
| User Port C / CIA2 PB0 | PA1 | Serial data |
| User Port D / CIA2 PB1 | PB2 / INT0 | Serial clock |

Connect the data, clock, reset-monitor, and optional JiffyDOS-control wires only to the matching labeled board points. The Ultra-36 board receives power and ground through the U36 socket. Optional 330 Ω to 1 kΩ series resistors may be used on the data and clock wires.

## Installation in the Commodore 128

1. Disconnect the computer from power and remove its case and shielding as required.
2. Locate the U36 function-ROM socket and note its pin-1 orientation before removing the existing device, if fitted.
3. Insert the programmed ATtiny84A and flash device into their sockets, matching every pin-1 indicator to the PCB markings.
4. Align the Ultra-36 connector with U36. Confirm that every pin enters the socket and that no row is offset.
5. Route the signal wires so they cannot be pinched by the shielding or case.
6. Recheck orientation, jumper position, power-to-ground resistance, and cable routing before reconnecting power.

## First power-on and test

On first power-up, bank 0 should display the Ultra-36 menu. `Empty_Bank` is bank 1, and user ROM labels start at bank 2. Select a known ROM and restart the computer to confirm bank switching and persistent selection. Holding RESET for approximately three seconds should temporarily return the selector to menu bank 0 without changing the saved EEPROM selection. A normal reset applies the saved EEPROM bank and JiffyDOS state again.

F5 to BASIC arms bank 1 temporarily without saving, then asks you to press RESET. The first reset enters the empty/BASIC bank; the following normal reset reads the EEPROM and returns to the saved ROM selection.

## IEC serial-bus warning

Do not leave two or more IEC disk drives or peripherals daisy-chained while they are powered off. Unpowered device logic can load or back-power the IEC Clock, Data, and peripheral RESET lines, causing C128 boot freezes, garbled screens, or missed Ultra-36 acknowledgements. One unpowered device may appear to work, but this is not guaranteed. Power every device that remains connected before starting the C128, or unplug every device that is off. This is an IEC-bus issue, not an Ultra-36 fault.

If the computer does not start, disconnect power immediately. Recheck U36 alignment, IC orientation, JP1, firmware fuse settings, flash contents, solder joints, and the User Port data and clock connections before trying again.

Remember that you modify your computer at your own risk.

## Ultra-36 on a budget

- Instead of an SST39SF040, you can use an AMIC A29040B. If you try to buy an SST flash device on AliExpress, you may receive a relabeled AMIC device.
- You can use standard pin headers as Ultra-36 mounting pins; rounded gold pins are recommended because they put less strain on the U36 socket.
- You can use standard-size resistors on Ultra-36, but you will need to bend them because the flash chip sits above them.
- You can use a standard 32-pin DIP socket instead of a precision socket, but you may need to file it so that it sits properly.

![Ultra-36 assembled with budget components](Ultra36_board_assembly_images/UltraBudget.jpg)
