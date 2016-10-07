==================================================
Texas Instruments Serial Boot Tool
==================================================
--------------------------------------
SerialBootTool - (For Version 1.3.0 or higher)
--------------------------------------
OtaServer communicates with the SerialBoot firmware on a device to send an image update.

This document assumes the user understands how SerialBoot firmware is loaded and used.  Each firmware target device comes with a document that describes how to do a serial boot load for that target.

This program supports three device families BLE, RF4CE, or Zigbee.

---------------------
Software Installation
---------------------
Run the setup.exe to install the software.

The software installer is composed of two files setup.exe and "SerialBootTool_Setup.msi".

A desktop shortcut, an entry for SerialBootTool, and the Readme file will appear under Texas Instruments in Start | Programs.

In the case SerialBootTool was included in another TI software release it will be installed by that package.

---------------------
Hardware Requirements
---------------------
A device loaded with the SerialBoot firmware.


==================================================
SerialBootTool Operation
==================================================
1. Select an image file.
2. Confirm serial port settings.
3. Confirm the device family is correct. (BLE, RF4CE, or Zigbee)
4. Press “Load Image”
5. A message will appear if the load was done successfully or if there was an error.




==================================================
FAQ
==================================================
1. The SerialBoot firmware works differently in each “Device Family”, please refer to the firmware documentation for building, loading and using the SerialBoot feature. 
2. A serial port open/close button is provided, but the serial port is automatically opened (if closed) when outgoing data shows up. For example pressing the “Load Image” button opens the port if closed.  When the load is complete the port is automatically closed.
3. There are tool tips for most screen objects, they contain help info and some cases extra information about items.
4. Using the Operations Log in the Options menu is a great way to debug issues and get some insight as to what the SerialBootTool is doing.



==================================================
Misc
==================================================
--------------------------------------------
Integrated Cross Platform Runtime Executable 
(Using Mono 2.10.5 or higher)
--------------------------------------------
1. Install SerialBootTool in Windows. (Note: Mono is NOT needed in Windows) 

2. Copy the installed files 
SerialBootTool.exe 
SerialBootTool.exe.config
SerialBootToolConfig.xml
as is, to a different OS with Mono 2.10.5 or higher installed.

3. Run "SerialBootTool.exe" using Mono on the different OS.

Tested using Windows XP, Windows 7, Ubuntu 12.04 LTS, Ubuntu 11.10.
Should be usable under most Mono compatable platforms!
See http://mono-project.com/What_is_Mono for more information. 

==================================================
Getting SerialBootTool Running In A OS Other Than Windows
==================================================

Install Mono 2.10.5 or higher
(ubuntu) sudo apt-get install mono

Install WinForms 4.0 for Mono
(ubuntu) sudo apt-get install libmono-system-windows-forms4.0-cil 

Using a terminal window, change to the directory where the files are and issue the following command:
mono SerialBootTool.exe


==================================================
More Information
==================================================

TI E2E -> http://e2e.ti.com/support/low_power_rf/f/159.aspx



