### Ultra-36 C128 Function ROM Menu ###

Ultra-36 is a Commodore 128 internal function ROM system that allows up to 8 selectable ROM banks (each 16KB or 32KB).
This project provides a bootable function ROM that displays a menu for selecting and identifying your ROMs, including support for a JiffyDOS toggle option.

Menu program built entirely in the CC65 toolchain, this system provides a robust, customizable launcher for embedded ROMs.

⸻

📂 Project Structure

Ultra36MenuRom/
├── build/               # Output: compiled ROM binary
├── cart128_16/          # Startup code & config for 16K function ROM
├── cart128_32/          # (Optional) 32K version (same menu code)
├── src/                 # C source code (menu and logic)
├── Makefile             # Build and run automation
└── README.md            # This file

🧰 Requirements
	•	CC65 compiler toolchain (must be in your PATH)
	•	VICE emulator with x128
	•	macOS/Linux or WSL (Windows) with GNU make

⸻

🚀 Building and Running

This repository will contain default compiled bin file in the build folder, you need to combine this with other roms listed in the Makefile.
Burn into recommended Flash Eprom. Note that Menu program will only work with Ultra-36 board for U36 socket in Commodore 128.

To compile and run the ROM directly in VICE C128 emulator (MacOs/Linux):

make run

This:
	•	Compiles src/main.c and startup files
	•	Produces build/cart128_16.bin
	•	Launches x128 with the ROM attached as a function ROM

To only build the ROM:

make

To clean build artifacts:

make clean

🧩 Customizing ROM Bank Labels

To change the ROM names shown in the menu, edit the DEFS section of the Makefile:

DEFS = -DROM1_NAME=\"GEOS1571\" \
       -DROM2_NAME=\"GEOS1581\" \
       -DROM3_NAME=\"Servant\" \
       -DROM4_NAME=\"SuperA\" \
       -DROM5_NAME=\"SuperB\" \
       -DROM6_NAME=\"Basic8\" \
       -DROM7_NAME=\"KeyDOS\"

You may use up to 7 labels. The 8th bank (Bank 0) is reserved for the Ultra-36 menu itself.

Use short names (6–10 characters) and escaped quotes as shown.

⸻

🧠 How It Works
	•	ROM code is placed at $8000–$BFFF (16K) or $8000–$FFFF (32K)
	•	Uses the C128 MMU to enable external cartridge bank
	•	Includes an interactive menu (arrow keys + enter)
	•	Optionally supports a JiffyDOS toggle in the UI
	•	All written in C and CC65 ASM using CC65 libraries

The final output cart128_16.bin can be:
	•	Flashed to SST39SF020A, SST39SF040 or similar EEPROMs
	•	Attached as a Generic C128 Function ROM in VICE
	•	Go to File > Attach cartridge image > Generic cartridge → Function ROM

🧑‍💻 Credits

Created as part of the Ultra-36 internal ROM selector project.

Designed to be simple, customizable, and expandable.





