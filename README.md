# PSOC&trade; Edge MCU: Bluetooth&reg; HFP Hands free

This application demonstrates a Hands-Free profile - Hands-Free (HFP-HF) using the AIROC&trade; Bluetooth&reg; stack. The application runs on the PSOC&trade; Edge E84 MCU Evaluation Kit and displays as an HF device. A remote mobile device can connect to it, allowing call controls.

The HFP-HF implementation supports essential telephony features, such as answering, ending, and redialing calls via onboard button actions. To ensure clear, full-duplex audio, the application incorporates a sophisticated Audio Front End (AFE). This includes Acoustic Echo Cancellation (AEC) and noise suppression, which prevent the microphone from recapturing audio from the onboard speakers effectively eliminating echo during active calls.

This code example has a three project structure: CM33 secure, CM33 non-secure, and CM55 projects. All three projects are programmed to the external QSPI flash and executed in Execute in Place (XIP) mode. Extended boot launches the CM33 secure project from a fixed location in the external flash, which then configures the protection settings and launches the CM33 non-secure application. Additionally, CM33 non-secure application enables CM55 CPU and launches the CM55 application.

[View this README on GitHub.](https://github.com/Infineon/mtb-example-psoc-edge-btstack-hfp-hf)

[Provide feedback on this code example.](https://yourvoice.infineon.com/jfe/form/SV_1NTns53sK2yiljn?Q_EED=eyJVbmlxdWUgRG9jIElkIjoiQ0UyNDI0MDYiLCJTcGVjIE51bWJlciI6IjAwMi00MjQwNiIsIkRvYyBUaXRsZSI6IlBTT0MmdHJhZGU7IEVkZ2UgTUNVOiBCbHVldG9vdGgmcmVnOyBIRlAgSGFuZHMgZnJlZSIsInJpZCI6Im1vaGFua3VtYXIucmFtYWtyaXNobmFwYWRtYUBpbmZpbmVvbi5jb20iLCJEb2MgdmVyc2lvbiI6IjEuMC4wIiwiRG9jIExhbmd1YWdlIjoiRW5nbGlzaCIsIkRvYyBEaXZpc2lvbiI6Ik1DRCIsIkRvYyBCVSI6IklDVyIsIkRvYyBGYW1pbHkiOiJQU09DIn0=)

See the [Design and implementation](docs/design_and_implementation.md) for the functional description of this code example.


## Requirements

- [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) v3.7 or later (tested with v3.9)
- Board support package (BSP) minimum required version: 1.0.0
- Programming language: C
- Associated parts: All [PSOC&trade; Edge MCU](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm) parts


## Supported toolchains (make variable 'TOOLCHAIN')
- Arm&reg; Compiler v6.22 (`ARM`)

## Supported kits (make variable 'TARGET')

- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC2`) – Default value of `TARGET`
- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC4`)


## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

Ensure the following jumper and pin configuration on board.
- BOOT SW must be in the HIGH/ON position
- J20 and J21 must be in the tristate/not connected (NC) position


## Software setup

See the [ModusToolbox&trade; tools package installation guide](https://www.infineon.com/ModusToolboxInstallguide) for information about installing and configuring the tools package.

- Install a terminal emulator if you do not have one. Instructions in this document use [Tera Term](https://teratermproject.github.io/index-en.html).

- Install the DEEPCRAFT™ Audio Enhancement (AE) Tech Pack to access the Configurator tool

This example requires no additional software or tools.


## Operation

See [Using the code example](docs/using_the_code_example.md) for instructions on creating a project, opening it in various supported IDEs, and performing tasks, such as building, programming, and debugging the application within the respective IDEs.

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector

2. Open a terminal program and select the KitProg3 COM port. Configure the serial port settings to 115200 baud, 8 data bits, no parity, and 1 stop bit (8N1)

3. Build and program the board with this application using ModusToolbox&trade;. After programming, the application starts automatically and displays the title "PSOC EDGE MCU: Handsfree Profile (HFP) Unit - Hands free (HF)"

   **Figure 1. Terminal output after HFP HF startup**

   ![](images/hfp_hf_startup.png)

4. The application initializes the Bluetooth&reg; acting as a Hands-Free (HF) device and start advertising with the name "Hands-Free Unit". The blue LED (user LED3) blinks to indicate that the device is ready to pair. Ensure you request pairing from the smartphone

   **Figure 2. Terminal output after HFP HF is ready to pair**

   ![](images/hfp_hf_advertisement_start.png) 

5. Once connected to an smartphone or AG device, the call control capabilities will be activated. A call can be initiated from the connected smartphone or AG device

   **Figure 3. Terminal output after HF device connected** 

   ![](images/hfp_hf_connection_complete.png)

6. An incoming call will cause ringing on the HF (with logs on the console)

   **Figure 4. Terminal output when HF have an incoming call** 

   ![](images/hfp_hf_incoming_call.png)

7. To accept the call, press and release **USER_BTN1 (SW2)**

   **Figure 5. Terminal output on HF accepting the incoming call** 

   ![](images/hfp_hf_incoming_call_accepted.png)

8. Once the call is accepted, the incoming voice audio can be audible on the evaluation kit (EVK)'s speaker and the voice audio captured from EVK's microphone would be sent over a call

9. To end the call, press and release **USER_BTN2 (SW4)**

   **Figure 6. Terminal output on HF ending the active call** 

   ![](images/hfp_hf_active_call_ended.png)

10. To initiate a call to a previous (redial) phone number in the smartphone's dial list, press and hold **USER_BTN1 (SW2)** for more than three seconds.

    **Figure 7. Terminal output on HF redial** 

    ![](images/hfp_hf_redial_call.png)


## Related resources

Resources | Links
----------|--------------------------
Application notes  | [AN235935](https://www.infineon.com/AN235935) – Getting started with PSOC&trade; Edge E8 MCU on ModusToolbox&trade; software <br> [AN236697](https://www.infineon.com/AN236697) – Getting started with PSOC&trade; MCU and AIROC&trade; connectivity devices <br> [AN240916](https://www.infineon.com/AN240916) – DEEPCRAFT&trade; Audio Enhancement (AE) on PSOC&trade; Edge E84 MCU
Code examples  | [Using ModusToolbox&trade;](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub
Device documentation | [PSOC&trade; Edge E84 MCU datasheet](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm/psoc-edge-e84#Documents) <br> [PSOC&trade; Edge E84 MCU reference manuals](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm/psoc-edge-e84#Documents)
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board)
Libraries  | [mtb-dsl-pse8xxgp](https://github.com/Infineon/mtb-dsl-pse8xxgp) – Device support library for PSE8XXGP <br> [retarget-io](https://github.com/Infineon/retarget-io) – Utility library to retarget STDIO messages to a UART port <br> [btstack](https://github.com/Infineon/btstack) – BTSTACK library <br> [btstack-integration](https://github.com/Infineon/btstack-integration) – The btstack-integration hosts platform adaptation layer (porting layer) between AIROC&trade; BTSTACK and Infineon's different hardware platforms <br> [kv-store](https://github.com/Infineon/kv-store) – This library provides a convenient way to store information as key-value pairs in non-volatile storage
Tools  | [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) – ModusToolbox&trade; software is a collection of easy-to-use libraries and tools enabling rapid development with Infineon MCUs for applications ranging from wireless and cloud-connected systems, edge AI/ML, embedded sense and control, to wired USB connectivity using PSOC&trade; Industrial/IoT MCUs, AIROC&trade; Wi-Fi and Bluetooth&reg; connectivity devices, XMC&trade; Industrial MCUs, and EZ-USB&trade;/EZ-PD&trade; wired connectivity controllers. ModusToolbox&trade; incorporates a comprehensive set of BSPs, HAL, libraries, configuration tools, and provides support for industry-standard IDEs to fast-track your embedded application development

<br>


## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.


## Document history

Document title: *CE242406* – *PSOC&trade; Edge MCU: Bluetooth&reg; HFP Hands free*

Version | Description of change
------- | ---------------------
1.0.0   | New code example 
1.1.0   | Added AFE and Noise Cancellation support
1.1.1   | Fixed ARM compiler warnings
<br>


All referenced product or service names and trademarks are the property of their respective owners.

The Bluetooth&reg; word mark and logos are registered trademarks owned by Bluetooth SIG, Inc., and any use of such marks by Infineon is under license.

PSOC&trade;, formerly known as PSoC&trade;, is a trademark of Infineon Technologies. Any references to PSoC&trade; in this document or others shall be deemed to refer to PSOC&trade;.

---------------------------------------------------------

(c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
This software, associated documentation and materials ("Software") is owned by Infineon Technologies AG or one of its affiliates ("Infineon") and is protected by and subject to worldwide patent protection, worldwide copyright laws, and international treaty provisions. Therefore, you may use this Software only as provided in the license agreement accompanying the software package from which you obtained this Software. If no license agreement applies, then any use, reproduction, modification, translation, or compilation of this Software is prohibited without the express written permission of Infineon.
<br>
Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A SPECIFIC USE/PURPOSE OR MERCHANTABILITY. Infineon reserves the right to make changes to the Software without notice. You are responsible for properly designing, programming, and testing the functionality and safety of your intended application of the Software, as well as complying with any legal requirements related to its use. Infineon does not guarantee that the Software will be free from intrusion, data theft or loss, or other breaches (“Security Breaches”), and Infineon shall have no liability arising out of any Security Breaches. Unless otherwise explicitly approved by Infineon, the Software may not be used in any application where a failure of the Product or any consequences of the use thereof can reasonably be expected to result in personal injury.
