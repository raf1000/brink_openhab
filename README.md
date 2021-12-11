# brink_openhab

This repository allows to integrate Brink Renovent HR with openHAB. The integration seems to be the most comprehensive one (all data including TSP is available).

The following "converter" shall be placed between Brink Renovent HR and openHAB:
1. Wemos D1 mini
2. Master OpenTherm Shield:  https://diyless.com/product/master-opentherm-shield

The converter communicates with Brink Renovent HR via OpenTherm protocol. The physical connection via two wire cable.

The converter comminicates with openHAB via MQTT protocol. The  physical connection via Wifi.

# Converter set up (Arduino):

* Install and run arduino IDE to (change the code if needed) compile and upload programs to Wemos D1 mini
* The arduino Brink_2 program is locate in .src library
* The arduino Brink_2 program uses the "OpenTherm.h" library specified at  https://github.com/ihormelnyk/opentherm_library. This library is a part of Arduino IDE library and provides OpenTherm communication protocol plus some methods for heating appliances. However it does not include methods nor variables for ventilation/Brink Renovnet appliances therefore you need to change the standard "OpenTherm.h" library based on the extended copy ( part changed commeted with "Brink Renovent HR" located at .src/lib. Or used the files from .src/lib instead of the standard "OpenTherm.h" library  

# openHAB set up:

* Install MQTT Broker (mosquitto) and install MQTT binding openHAB.
* Items
* Sitemap



