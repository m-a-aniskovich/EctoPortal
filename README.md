
# EctoPortal

This repository contains the code and PCB for a modification to the classic model Ecto-1 using an Arduino IDE and an ESP-12 module.

## Features

-   Connects to a wifi network as a client using the ESP8266WiFi library
-   Creates a soft access point (AP) for configuration using the ESP8266WebServer library
-   Uses the ESP8266mDNS library to respond to hostname queries (myHostname = "ecto-1")
-   Stores wifi credentials in EEPROM using the EEPROM library
-   Handles DNS server using the DNSServer library

## How to Use

1.  Clone or download the repository to your computer
2.  Open the project in the Arduino IDE
3.  Update the constants at the top of the code with your desired soft AP credentials (APSSID and APPSK)
4.  Update the IP address, netmask, and other network configurations as per your requirement.
5.  Connect your ESP-12 module to your computer and upload the code to the board
6.  Once the code is uploaded, the ESP-12 module will start in soft AP mode, you can connect to the AP using the SSID and password you set in step 3
7.  Once connected to the soft AP, you can configure the wifi credentials for the client mode and the ESP-12 will connect to the wifi network
8.  Once connected to wifi network, you can check the board's IP address by checking your router's DHCP client list or by using mDNS hostname resolution.

## Contributions

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](https://chat.openai.com/LICENSE) file for details.
