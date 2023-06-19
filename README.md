# brink_openhab

This repository allows to integrate Brink Renovent HR with openHAB. The integration seems to be the most comprehensive one (all data including TSP is available). If you have another model ex. Brink Renovent Excellent the integration method and attached programs are the same but some of avaiable parameters are different, located in different places (TSPs) or not available.

You need to place  a "converter" between  Brink Renovent HR ( opentherm voltage of 7..15V) and OpenHAB (ESP8266 voltage of 3..5V).
I have used the following one:
1. Wemos D1 mini
2. Master OpenTherm Shield:  https://diyless.com/product/master-opentherm-shield

The converter communicates with Brink Renovent HR via OpenTherm protocol. The physical connection via two wire cable.

The converter communicates with openHAB via MQTT protocol. The physical connection via Wifi.

# Converter set up (Arduino):

* Install and run arduino IDE, connect to Wemos D1 mini via USB.
* Upload brink_hr program: [Brink_HR.ino](https://github.com/raf1000/brink_openhab/blob/main/Arduino/Brink_HR.ino) or [Brink_HR_bypass.ino](https://github.com/raf1000/brink_openhab/blob/main/Arduino/Brink_HR_bypass.ino) to arduino IDE
* Specify network and mqtt parameters in the Brink_HR program. Perform changes in the code if you want addtional TSP to be avilable.
* The additional libraries shall be installed as specified in the program including: *<OpenTherm.h>*, *<ESP8266WiFi.h>*, *<PubSubClient.h>*

<OpenTherm.h> library specified at  https://github.com/ihormelnyk/opentherm_library is a part of Arduino IDE library and provides OpenTherm communication protocol plus few methods for heating appliances. However it does not include methods nor variables for ventilation/Brink Renovent appliances. Therefore you need to use the following files which includes Brink Renovent HR relevant additions:
Updated [OpenTherm.h](https://github.com/raf1000/brink_openhab/blob/main/Arduino/OpenTherm.h)
Updated [OpenTherm.cpp](https://github.com/raf1000/brink_openhab/blob/main/Arduino/OpenTherm.cpp)
You can use the above files directly instead of the original <OpenTherm.h> library or make changes to the arduino IDE library (copy parts commeted as "Brink Renovent HR" from updated files). In order to do that you need to find directory where theses files are stored in your Arduino IDE, typical path is:  ...\Arduino\libraries\OpenTherm_Library\src\.
 
 * Upload the compiled Brink_HR_Bypass progrm to wemos D1 mini. Connect wemos D1 mini to the converter and Brink Renovent HR

# openHAB set up:

* Install and set up MQTT Broker (mosquitto)
* Install MQTT binding in openHAB 
* Create items :  [brink.items](https://github.com/raf1000/brink_openhab/blob/main/openHAB/brink.items) file
* Create MQTT client with channels : [Brink_MQT_Client](https://github.com/raf1000/brink_openhab/blob/main/openHAB/Brink_MQTT_Client) file
* Update sitemap : [brink.sitemap](https://github.com/raf1000/brink_openhab/blob/main/openHAB/brink.sitemap) file /* includes items for additional sensors*/


After successful setting you shall see something similar to:

![Openhab brink](https://github.com/raf1000/brink_openhab/blob/main/RenoventHRLarge.jpg)


# Some notes:
1. According to OpenTherm protocol version 2.2 specification:
-   *The master must communicate at least every 1 sec (+15% tolerance).* 
                          This is probably true for a heating appliance but in case of connection with Brink Renovent only I have noticed that communication works also when conversation frames are sent with longer interval (ex 10 sec). In the program I have set OT communcation frequency at 1.5 sec.
-   *It is required that OpenTherm-compliant devices support the following data items (Message Ids): ID=0, ID=1, ID=3.*
                             This is probably true for a heating appliance but in case of connection with Brink Renovent only these messages are not needed (and Brink as a ventilation device does not recognise them)   
2.  In order reduce traffic of incoming events in openHAB, the Converter sends message to openHAB only upon "significant" changes of parameters/values in Brink. For example current pressure in input/output duct is changing constantly by 1Pa but the converter udpates the current pressure value in openHAB only if the change is bigger than 2Pa.
3.  I have managed to identify 61 TSPs (user/installer available parameters) which are specified in the updated <OpenTherm.h> file with indication of 2 bytes parameters (a parameter stored in 2 TSPs). All TSPs parameters can be read and changed (if allowed) using the following methods:
- *getBrinkTSP(parameter)/setBrinkTSP(parameter)*   for 1byte parameters (ex. U4, U5, I1 ..)
- *getBrink2TSP(parameter)/setBrink2TSP(parameter)* for 2bytes parameters (ex. U1, U2, U3, Max Vol, CPOD, CPID, .. )

* I have only U1, U2, U3, U4, U5 and I1 as read/write. In my case changes of other Brink`s adjustable parameters in openHAB are not practical (for example I do not have a preheater). But if needed you can update the .ino program (then add channels and items in openHAB) to read/write them as well.
6.  I have noticed that Opentherm protocol connection can work in parallel with multiple switch (3-way switch). After switching from Step 1 to Step2/3, the 3-way switch overrides the ventilation capacity set up by OpenTherm. In such case the 3-way swith force Brink to run according to U2 or U3 parameter and all other parameters (ex preassure, temperature,.. ) are available.
7.  After restarting openHAB it might be needed to reset the converter in order to populate slow changing parameters in openHAB (unless you want to use "restore" in openHAB). Repopulation of parameters can also be done by any change of U5 parameter visible in sitemap.
8.  Seems that none of the standard bits (LB2, LB3, HB1, HB2) of the Msg=70 in OT protocol is updated by Brink Renovent when bypass position is changed. But I have identified a TSP which conveys such information and it is implemented in the .ino program.

# Automatic Bypass (mounted as standard)

I have noticed a failure of automatic bypass while working with opentherm protocol. When opentherm is connected to Brink Renovent HR the automatic bypass is always closed despite that U4 and U5 conditions are met. Even if you manage to open it (opentherm not connected), then ~ 30 secondes after connecting opentherm bypass is closing despite that the system message informs that the bypass is open :-(

I have elaborated the following workaround (included in the Brink_HR_bypass.ino file):

(*) when bypass shall be open (U4 and U5 conditions are met) opentherm is connecting to Brink Renovent HR periodically
- parameter *readPeriod_bypass* shall be set to 20 seconds minimum(20 000)
- in the program I have set up 2 minutes for this parameter, which means that every 2 minutes your info about Brink parameters will be read from Brink  and updated in openHAB
- writing/changing values in Brink are on-line (but you may need to wait until a change is visible)

(**) in such a periodic connection mode you shall NOT control ventilation capacity using ot.setVentilation() methood but only by changing U1 parameter. 
- it is important to ensure that both parameter U1 and ot.setVentilation() are set at the same level to avoid frequent changes of Brink`s ventilators speed
- as an example I am attaching my openHab rules in which Brink Renovent flow is controled based on Home Humidity sensor for both open and closed bypass.










