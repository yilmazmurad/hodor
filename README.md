PN532 RFID Reader Project
Overview

This project demonstrates the use of the PN532 RFID/NFC module with Arduino. It allows the reading of RFID tags and managing authorized access through the use of predefined UID cards stored in EEPROM. The project is designed to add new cards, check authorized cards, and reset the EEPROM when needed.
Features

    Read RFID Tags: Detect and read RFID tags using the PN532 module.
    Authorized Access Control: Check if the scanned RFID tag is authorized.
    Card Management: Add new cards to the authorized list and reset the EEPROM.
    EEPROM Storage: Store authorized card UIDs persistently using EEPROM.

Components Required

    Arduino UNO or compatible board
    PN532 RFID/NFC module
    Jumper wires
    Relay module (optional, for access control)

Wiring Diagram

Here’s how to connect the PN532 module to your Arduino:
PN532 Pin	Arduino Pin
SDA	21
SCL	22
VCC	5V
GND	GND
Installation

    Libraries: Install the following libraries in your Arduino IDE:
        Adafruit PN532
        EEPROM

    Clone the Repository:

    bash

    git clone https://github.com/yilmazmurad/hodor.git
    cd hodor

    Upload the Code: Open the main.cpp file in your Arduino IDE, and upload it to your Arduino board.

Usage

    Initial Setup:
        Make sure the PN532 module is connected properly.
        Upload the code to your Arduino.

    Adding Cards:
        Scan the master card to enter the card addition mode.
        Present the new card you want to add.
        The card will be added if it is not already in the list.

    Resetting EEPROM:
        Present the reset card to clear all authorized cards stored in EEPROM.

    Testing Authorized Access:
        Scan any card to check if it’s authorized. The relay will be activated for authorized cards.

Code Structure

The main functionalities are handled in the following sections:

    Setup:
        Initialize the PN532 module and EEPROM.
        Load any pre-existing cards from EEPROM.

    Loop:
        Continuously check for new cards and handle the logic for adding, resetting, and checking cards.

Troubleshooting

    Ensure that the PN532 module is powered correctly and connected to the correct pins.
    Check the baud rate settings if you're having issues reading cards.
    If the RFID tags are not detected, try bringing them closer to the PN532 module.

License

This project is licensed under the MIT License - see the LICENSE file for details.
Acknowledgments

    Adafruit for providing the PN532 module and related libraries.
