# RotorHazard ESP8266 Connector

This project, also known as "RotorHazard ESP8266 Connector", is a firmware for ESP8266. It connects via websockets (WiFi) to a RotorHazard instance. RotorHazard is a Race Timer for FPV drones which tracks the time one needs to finish a lap in a race and automatically tracks the race with up to 8 pilots. The ESP is connected via a serial UART to the flight controller of the drone and sends "MSP" commands to set the internal name of the drone to messages that represent events that happen in the race timer.

## Installation

1. **Embark on the PlatformIO Journey**: Your first mission is to install PlatformIO, an open-source ecosystem for IoT development. Fear not, for the [PlatformIO documentation](https://docs.platformio.org/en/latest/core/installation.html) will guide you on this journey.

2. **Summon the Repository**: With PlatformIO in your toolkit, it's time to clone this repository. Cast the following incantation in your terminal: `git clone https://github.com/UAV-Painkillers/rotorhazard-msp-osd-injector`. 

3. **Connect the ESP8266 Board**: Now, prepare your ESP8266 board for the adventure ahead. You can use an ELRS receiver or any other compatible ESP8266 board. Connect it to your PC using an FTDI adapter. If you're using an ELRS receiver, remember to bridge the boot and GND pin for flashing. Alternatively, you can use another ESP8266 development board like the Wemos D1 Mini. Bridge the boot pin with GND and connect the TX and RX pins to the TX and RX pins of the receiver. Your development board is now an FTDI adapter.

4. **Choose Your Target**: In the realm of PlatformIO, select the right target. Seek one that ends with `_uart`. Beware of the `_ota` targets, they're elusive and will only work after you've flashed it the first time.

5. **Launch the Firmware**: The final step is to hit the upload button in PlatformIO. This will flash the firmware to your ESP8266 board. Once the upload is complete, your board is ready to join the race with the RotorHazard ESP8266 Connector.

6. **Install Node.js**: Before proceeding, ensure that Node.js is installed on your system. If not, follow the official [Node.js installation guide](https://nodejs.org/en/download/package-manager/).

7. **Install Bunjs**: This project uses Bunjs as a runner and dependency manager. If you haven't installed it yet, follow the official [Bunjs installation guide](https://bun.js.org/guide/getting-started.html#installation).

8. **Build the Web UI**: Open a terminal, navigate to the "web-ui" subfolder using the command `cd web-ui`, and then run the command `bun run build`.

9. **Build and Upload Filesystem Image**: Open the PlatformIO side menu in VS Code (the alien icon), click on "Build Filesystem Image", and wait for the process to complete. After that, click on "Upload Filesystem Image".

## Usage

1. **Install on a Drone**: To begin, you MUST install the software on a drone. Connect the TX pin of the receiver to a free UART RX pad or a softserial RX pad on the drone. Note that the RX pad of the receiver is not used.

2. **Configure Betaflight**: Open Betaflight and navigate to the ports configuration tab. Enable MSP for the port and set the baud rate to 57600.

3. **Add the Craft Name OSD Element**: This step is CRUCIAL. Without adding the craft name OSD element, we cannot display any information in the OSD.

4. **Power Up**: Now that the hardware setup is complete, power up the drone and your FPV goggles. The unconfigured module will launch a Wi-Fi hotspot named "rotorhazard-osd-injector" (password: "rotorhazard"). You should see messages in the OSD indicating that the hotspot is configured and enabled.

5. **Connect and Configure**: Connect to the hotspot and open "http://10.0.0.1" in a browser. You'll be greeted with an authentication dialog. To change any settings, you need to authenticate. Click on the "Show Pin (OSD)" button, and a message will appear in the OSD with the pin. Enter the pin to proceed.

6. **Set Wi-Fi Credentials**: Enter your Wi-Fi credentials and click on the "Connect Wi-Fi" button. The hotspot will disappear, and you should connect your PC to your Wi-Fi. Check the OSD for the IP of the module and note it down. Open the IP in your browser in the following format: http://<ip>.

7. **Configure Hotspot (Optional)**: You can optionally set the name (SSID) and password of the hotspot, which automatically starts when a connection to the configured Wi-Fi fails for 30 seconds.

8. **Configure RotorHazard Settings**: Enter the hostname and port for RotorHazard (default port is 5000). You can also optionally set the SocketIO path, but the default value is usually fine.

9. **Load Pilots**: After saving the connection details, click on the "Reload Pilots" button to load the saved pilots from RotorHazard. If the list does not update, try reloading the page.

10. **Select Pilot**: Select the pilot you want to receive notifications for (probably yourself!) and click on "Save selection".

If everything is set up correctly, you should start seeing notifications about RotorHazard connection, completed laps, etc., in your OSD.

## Prerequisites

You need an ESP of the ESP8266 family. The simplest, smallest, and most lightweight option is to buy a "BetaFPV ELRS RX Lite" receiver which has an ESP8285 onboard and exactly one UART available for communicating with the drone.

## Contributors

Jan Jaap ([GitHub: JappyJan](https://github.com/JappyJan), Email: mail@janjaap.de)

## License

This project is licensed under a license that allows everyone to contribute and make it better, create forks of it, and use it for their own projects. However, it does not allow anyone to sell this code or make any other sort of profit from it. The specific license will be added later.