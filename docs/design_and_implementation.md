[Click here](../README.md) to view the README.

## Design and implementation

The design of this application is minimalistic to get started with code examples on PSOC&trade; Edge MCU devices. All PSOC&trade; Edge E84 MCU applications have a dual-CPU three-project structure to develop code for the CM33 and CM55 cores. The CM33 core has two separate projects for the secure processing environment (SPE) and non-secure processing environment (NSPE). A project folder consists of various subfolders, each denoting a specific aspect of the project. The three project folders are as follows:

**Table 1. Application projects**

Project | Description
--------|------------------------
*proj_cm33_s* | Project for CM33 secure processing environment (SPE)
*proj_cm33_ns* | Project for CM33 non-secure processing environment (NSPE)
*proj_cm55* | CM55 project

<br>

In this code example, at device reset, the secure boot process starts from the ROM boot with the Secure Enclave (SE) as the root of trust (RoT). From the Secure Enclave, the boot flow is passed on to the system CPU subsystem, where the secure CM33 application starts. After all necessary secure configurations, the flow is passed on to the non-secure CM33 application. The non-secure CM33 application then enables the CM55 core using the `Cy_SysEnableCM55()` function. Resource initialization for this example is performed by this CM55 project. It configures the system clocks, pins, clock to peripheral connections, and other platform resources.

The CM55 project performs the following:

1. Initializes the AIROC&trade; Bluetooth&reg; stack (BTSTACK) for the PSOC&trade; Edge E84 MCU

2. Configures and registers the Hands-Free Profile (HFP) Hands-Free (HF) service with the Bluetooth&reg; stack

3. After connection, it initializes UART logging, GPIOs for buttons, and other peripherals required for the HFP functionality

4. Starts Bluetooth&reg; advertising and waits for an audio gateway (AG) (i.e. Smartphone) device to connect

5. Allows answering an incoming call using **USER_BTN1** (short press)

6. Establishes a synchronous connection-oriented (SCO) audio connection to enable two-way voice communication, where an incoming voice from the smartphone is played through the evaluation kit's (EVK) speaker and outgoing voice captured by the EVK’s microphone is sent to the smartphone

7. Ends the call when **USER_BTN2** is short pressed. which terminates the SCO audio connection

8. Allows initiating call to a phone number that is available on top of the smartphone's dial list (Redial) when **USER_BTN1** is long pressed

The buttons, such as **USER_BTN1** and **USER_BTN2** on the board have been set up to generate an interrupt upon being pressed, and it can perform the following functions when pressed.

- **USER Button 1 (SW2)**

  **Short press**
  Send the *Call Accept* command to the connected smartphone.

  **Long press (more than three seconds)** 
  Send the *Redial* command to the connected AG device, which will trigger a call to the phone number present on top of the smartphone's dial list.


- **USER Button 2 (SW4)**

  **Short press**  
  Send the *Call End* command to the connected smartphone.

**DEEPCRAFT&trade; Audio Enhancement (AE) configuration**

- To ensure clear, full-duplex audio, the application incorporates a sophisticated Audio Front End (AFE). This includes Acoustic Echo Cancellation (AEC) and noise suppression, which prevent the microphone from recapturing audio from the onboard speakers effectively eliminating echo during active calls.

- The DEEPCRAFT&trade; Audio Enhancement (AE) modules are chosen via the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator tool. Use the DEEPCRAFT&trade; Audio Enhancement (AE) Configurator project file present at path *proj_cm55\source\HFP_BT\audio_enhancement_application/hfp_hf_ae.mtbafe*.

- Refer to [Audio Enhancement (AN240916)](https://www.infineon.com/AN240916) – DEEPCRAFT&trade; Audio Enhancement (AE) for understanding about the Audio Enhancement (AE) and tuning.
  
The application consists of the following functionality.

| Function                                                          | Core |
| :-------------------------------------------------------          | :--- |
| Default retarget_io for printing logs using kitprog UART          | CM55 |
| Main application task - handles BT connection/disconnection       | CM55 |
| Bluetooth Stack receive/transmit Tasks                            | CM55 |
| Hands Free call handling                                          | CM55 |

<br>