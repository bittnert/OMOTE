# Configuration Design Overview

The Omote remote control is a universal remote control which can combine the control of many different devices into one remote control.

This document tries to layout an operational concept how the remote control could be setup to provide enough adaptability to be useful for a broad range of users while being easy enough to use so no programming knowledge is needed to get the remote configured and adapted to the specific use cases.

## Basic terminology

This chapter tries to explain some basic terminologies as well as basic concepts on how the remote could be configured.

The following basic building block will be explained:

* [Device Type](#devices-type) Not visible to normal users except when a new, unknown, devices is added.
* [Devices](#devices) Most basic building blocks.
* [Action](#action) Combination of devices to create some defined behavior.
* [Scene](#scene) Combination of Actions and devices which additionally represent a state machine to modify button behavior.

### Devices Type

The device type is the only thing that needs to be programmed into the firmware. This basically describe how to communicate with a specific device. If a device type is supported, all devices that use that device type are automatically supported by the firmware.

Examples for device types would be

* IR encoding like the RC5 or NEC protocol [see](https://www.digikey.com/en/maker/tutorials/2021/understanding-the-basics-of-infrared-communications)
* WiFi protocols like MQTT

The Device configuration would then provide the device type so the remote knows how to talk to the device.

### Devices

Devices represent the most basic type that can be configured. Every device is assigned a device type which tells the remote how to communicate with the device. Basically, a device defines what each button does (which IR code or wifi command is send out) and which additional buttons on the touch screen are available.

For example, a TV or a projector as well as some home automation like automated lighting could be a device.

If a device on the remote control is selected, all buttons only control this one device. The use case for selecting an specific device and only controlling this one device will be an edge case and only used very seldom but it still needs to be suported.

### Action

An Action represents a combination of command being send to different devices.

For example, one action could consist of the following steps:

* Turn on projector
* Role down the projector screen
* Close the automated blinds on the windows
* turn on the video source
* turn on the Audio system

Inside the an Action, the device and the command together with a timeout is specified so the remote control has just a list of commands to execute. The execution of an action does not change the meaning of any other buttons on the remote control but only preform certain steps in an certain order.

### Scene

*I don't like this naming, maybe we can come up with some better name*

A Scene would use Devices and Actions and group them together. Specifically, when a Scene is selected, some sort of state machine would be added to the remote control.

For example, there could be a Scene for watching some Netflix (or similar) on a projector. When this Scene is selected, the starting Action would be the one described above where the projector, screen, video source and audio system is turned on.

However, in addition the buttons on the remote are remapped. For example, the volume keys would send out IR codes for the Audio System while the arrow keys would send out signals to the video source (e.g. WiFi commands to a FireTV stick or IR signals to a computer or similar).

When then the off button is pressed, it would send the following commands

* Tun off the Audio system
* Turn off the video source
* Open the automated blinds on the windows
* Role up the projector screen
* Turn off the projector

The idea here is, that this is the typical use case. If someone wants to watch some film or play video games, the Scene can be started and then the remote control already knows what command to send where in order to control the different aspects.

## Concept for remote configuration

This chapter will first describe the basic limitation that should be considered when deciding on the configuration structure for the remote as well as the different concepts that can be used to configure the remote.

The configuration structures shown below are only meant to show the basic idea. The exact size of the different fields and information encoded in specific bit positions is not documented (yet).

### Basic Consideration of the configuration structure

As the configuration structure is only created once but read/used by the remote possibly on a daily basis, the structure should emphasize to be easy (less resource intensive) to read/use.

Additionally, the structure should be stored in a binary format, as the hardware of the remote control is very limited in terms of memory and CPU performance. This would save memory on Flash (and maybe RAM) and easy the reading of the data for the CPU as not translation from a string based configuration has to be done.

As the exact size of the configuration structure depends on the configured Devices/Actions/Scene, it must contain information on how big the data is.

Do improve reliability, the configuration structure should contain a version which indicates the structure of the configuration structure (so the firmware knows if it can understand the configuration or not) as well as a CRC32 (or similar) to ensure the integrity of the configuration structure before it is used.

Either one configuration structure which contains all Devices/Actions/Scene or three separate configuration structures are possible. If three different structures are used, three different version would need to be used.

In this document, the three configuration structures will be described individually. However, as the structures might reference each other, it is recommended to combined them into one structure so they are consistent.

The configuration structure needs to contain strings as well. Strings need to be added at different points within the structure. Because the strings need to be shown on a relatively small screen on the remote control, it would make sense to limit the maximum string length to ensure the string can be read on the screen.

*There are two possible solution to include strings in the structure.

*Either null terminated strings can be included in the structure where it is need, or appending null terminated strings to the end of the configuration structure, and then referencing the strings from within the structure.*

*The first option should be easier to implement while the second option can save some memory as strings might be shared between devices.*

## Device configuration structure

The device configuration structure has to include the following information:

* Device name
  * String to identify the device.
* Device type
  * For example IR encoding or WiFi protocol. See [[#Devices Type]]
* Optional additional device type information
  * For example for WiFi protocol this could be the IP address and or Port of the device
* Codes to be send out for the Physical buttons
  * The number of buttons is fixed, however, the length of the codes depend on the device type (e.g. 16 bit for IR or longer for WiFi protocols)
* Virtual buttons and the associated codes
  * Each virtual button has to contain a string to put on the virtual button and the code to be send when the virtual button is pressed

The Figure below shows the principle build up of a device configuration structure.

![[device_config.png]]

## Action configuration structure

The action configuration structure has to contain the following data:

* Name of the Action
  * String which is used to identify the action for the user
* List of following information
  * Devices ID
    * This ID references to the device in the device configuration structure. Device ID 0 would refer to the first Device in the Device config. Device ID 1 would refer to the second Device in the device config etc.
  * Button to be pressed
    * The button to be pressed would refer to the position of the button in the Device configuration
  * Timeout to be kept
    * Timeout in ms to wait after the button was pressed

The figure below shows the structure of the Action configuration.

![[Action_config.png]]

It would also be possible to have an Action which does not have an Action name (in this case the name could be a string of length 0). These actions might be needed/used for Scenes but they can not be triggered by the user directly.

## Scene configuration structure

The Scene configuration will use the Device and Action configuration to provide specific functionality where buttons are mapped to different, specific devices and buttons.

The following information needs to be provided for each Scene defined

* Name of the Scene
  * String for the user to identify the Scene
* Power Up Action
  * An Action ID (or Device and Button ID) which has to be executed when Scene is started
* Power Down Action
  * An Action ID (or Device and Button ID) which has to be executed when the Scene is stopped
* For each physical button
  * An Action ID or Device and Button ID to be mapped to the specific physical button
* For each virtual button
  * Name of the virtual button
  * An Action ID or Device and Button Id to be mapped to the virtual button

The Figure below shows the the structure of such a Scene configuration

![[Scene_config.png]]

## Configuration structure creation

As the structure is not human readable and relatively complex, a tool to create and upload such a configuration structure is needed. For such a tool, several requirements/limitation can be defined.

1. The tool shall be easy to use, also for non technical people
    1. Therefore a GUI tool is needed
2. The tool shall be easily accessible
    1. We should not be forced to rely on some download page to provide the tool which people first have to download
3. The tool shall provide an easy way to program the configuration onto the remote
    1. A simple press of the button should be sufficient to create and upload the configuration
4. The tool shall be cross platform
    1. As people might use different operating systems or computer systems (x86, ARM, Windows, Linux, MacOS), the tool should not be limited to only a subset of these
5. (optionally) the tool shall be usable from a mobile phone

Because of these requirements listed above, a possible solution would be to use some web server hosted on the remote it self.

If someone wants to configure the device, the remote could show an QR code and the IP address of the device (or maybe a zero conf domain). By scanning the QR code or entering the IP address/domain in a browser, the web side could be loaded directly from the remote control.

It is possible to store the configuration page as html/js/css files in flash and the esp32 would only provide this files directly. So while the interface is used, the remote would not be involved and everything would be calculated on the users PC.

One big advantage of this approach is, that we can use the superior CPU and memory resources of the users PC (or mobile phone) to create the configuration. Additionally, the interface could also access some database on the internet which might already contain devices ready to be used.

When a configuration is done, a click of a button would be sufficient for the web interface to know its server (the remote) and send the configuration structure there. This way, it would even be possible to have several remote controls on the same network and know exactly which one is being configured.

One disadvantage of this approach is, that the configuration tool is stateless. Which means, if the side is closed and opened again, the tool does not remember what was done last. This could be helped by being able to create some human readable configuration file (maybe JSON or toml or xml or similar) which can be downloaded and stored to be able to restore/backup different configurations. Additionally, it might be possible to load the binary configuration structure from the remote and decode so it is possible to only modify the current configuration without re-creating it from scratch.

*Which frame work and programming language to use would need to be determine, however javascript might not be a good choice as it does not support creation of binary data (javascript does not have integer type, only "number") Therefore, web assembly or similar might be needed.*
