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
* The arduino Brink_2 program uses the "OpenTherm.h" library specified at  https://github.com/ihormelnyk/opentherm_library. This library is a part of Arduino IDE library and provides OpenTherm communication protocol plus some methods for heating appliances. However it does not include methods nor variables for ventilation/Brink Renovnet appliances therefore you need to change the standard "OpenTherm.h" library based on the extended copy ( part changed commeted with "Brink Renovent HR") or used directly the files from this repository instead of the standard "OpenTherm.h" library.

# openHAB set up:

* Install MQTT Broker (mosquitto)
* Install MQTT binding in openHAB 
* Create items :  [brink.items](https://github.com/raf1000/brink_openhab/blob/main/openHAB/brink.items) file
* Create MQTT client with channels : [Brink_MQT_Client](https://github.com/raf1000/brink_openhab/blob/main/openHAB/Brink_MQTT_Client) file
* Update sitemap : [brink.sitemap](https://github.com/raf1000/brink_openhab/blob/main/openHAB/brink.sitemap) file /* includes items for additional sensors*/
*

# Some notes:
1. Accoroding to OpenTherm protocol version specification 2.2 :
    (a) The master must communicate at least every 1 sec (+15% tolerance) . 
            This is probably true for a heating appliance but in case of connection with Brink Renovent only I have noticed that communication works also when a conversation  
            frames are sent with longer interval (ex 10 sec). In the program I have set 1.5 sec.
    (b) It is required that OpenTherm-compliant devices support the following data items: ID=0, ID=1, ID=3. 
            This is probably true for a heating appliance but in case of connection with Brink Renovent only these messages are not needed (and Brink as an ventilation device               does not reocgnise them)   
2.  
