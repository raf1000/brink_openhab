/* Openhab - Brink Renovent HR Integration
Author: Raf1000
December 2021
*/

#include <Arduino.h>
#include <OpenTherm.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// this part schall be changed ----------------------------------
const char* ssid = "SET SSID of YOUR NETWORK";
const char* password = "PASSWORD";

const char* mqtt_server = "SET MQTT IP"; 
const char* mqtt_client_id = "brink"; // Unique client id
const int mqtt_Port = 1883; 
const char* mqtt_User = "SET MQTT USER"; 
const char* mqtt_Password = "SET MQTT PASSWORD"; 
const float maxVent = 2.49; //it means 249 m/h3 - max avilable flow in my Brink - NOT USED
const unsigned long readPeriod = 1500; // OT protocol requires master to communicate at least every 1 sec (+15% tolerance). 
// ---------------------------------------------------------------
const char* mqtt_topic_in = "brink/+/set"; //subscribe commands from Openhab

unsigned long startTime;
unsigned long currentTime;


const int HWCPin = 14; //Option: HW circulation pump D5
const int inPin = 4; //ESP8266 D2
const int outPin = 5; //ESP8266 D1 
OpenTherm ot(inPin, outPin);
WiFiClient espClient;
PubSubClient mqttClient(espClient);


float tIn, tIn_old = 0; // temp external
float tOut, tOut_old = 0; //temp internal
bool fault, fault_old = 1; 
bool vmode, vmode_old = 0;
bool sbypass, sbypass_old = 1;
//bool pbypass, pbypass_old;
bool filter, filter_old = 1;
int gvent, gvent_old = 0;
int pressin, pressin_old = 0;
int pressout, pressout_old = 0; // uint8_t
int vstep1, vstep1_old = 50;
int vstep2, vstep2_old = 150;
int vstep3, vstep3_old = 300;
int tU4, tU4_old = 1; //atmosphee temp U4
int tU5, tU5_old = 1; //inside temp U5
int cvol, cvol_old = 0;
int RPMin, RPMin_old;
int RPMout, RPMout_old;
int fcode, fcode_old = 0;
int msg, msg_old = 0;
int param1, param1_old = 100;
long lRssi, lRssi_old = 0;


void ICACHE_RAM_ATTR handleInterrupt() {
    ot.handleInterrupt();
}


void MqttReconnect() {  
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT Broker "); 
    Serial.println( mqtt_server );
    delay(1000);
    if (mqttClient.connect(mqtt_client_id,mqtt_User,mqtt_Password)) { 
      Serial.print("connected, topic: "); 
      mqttClient.subscribe(mqtt_topic_in);  
      Serial.println( mqtt_topic_in );
    } else {
      Serial.print("failed: ");
      Serial.print(mqttClient.state());
      Serial.println(" try again...");
      delay(5000);
    }
  }
  Serial.println(" ok...");
}



void MqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = 0;  

// Changing selected Brink Renovent HR parameters requested by OpenHab
  if(strcmp(topic, "brink/VentNomValue/set") == 0)  ot.setVentilation( atoi((char *)payload) );  // uint8_t

  if(strcmp(topic, "brink/U4/set") == 0) ot.setBrinkTSP(U4, atoi((char *)payload)*2 );
 
  if(strcmp(topic, "brink/U5/set") == 0) {
    ot.setBrinkTSP(U5, atoi((char *)payload)*2 );
    refreshAll(); // change of U5 triggers refresh of other paramteres
  }

// HW Circulation pump swith - option
  if(strcmp(topic, "brink/HWCP/set") == 0)  {
      if ( *payload == '1' ) digitalWrite(HWCPin, LOW); // realy switch on
      if ( *payload == '0' ) digitalWrite(HWCPin, HIGH); // realy swich off
  }
}

void setup()
{ 
    Serial.begin(115200);
    Serial.println("Start");
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.print("IP Addresss :");
    Serial.print(WiFi.localIP());

    mqttClient.setServer(mqtt_server, mqtt_Port); 
    mqttClient.setCallback(MqttCallback);
    MqttReconnect();

    startTime = millis(); 
    ot.begin(handleInterrupt);
    
// Floor heting pump switch - option
    pinMode(HWCPin, OUTPUT); 
    digitalWrite(HWCPin, HIGH); 
}

// refresh slow changing parameters on request by every change of U5
void refreshAll()  
{ 
    
    mqttClient.publish("brink/TempSupplyIn/get", String(tIn).c_str());

    mqttClient.publish("brink/TempExhIn/get", String(tOut).c_str());
     
    mqttClient.publish("brink/FauiltIndication/get", String(fault).c_str()); 

    mqttClient.publish("brink/VentilationMode/get", String(vmode).c_str());
 
    mqttClient.publish("brink/BypassStaus/get", String(sbypass).c_str());
  
    mqttClient.publish("brink/FilterDirty/get", String(filter).c_str());
 
    mqttClient.publish("brink/VentilationNominalValue/get", String(gvent * 2.48).c_str());
   
    mqttClient.publish("brink/CurrentVolume/get", String(cvol).c_str());
 
    mqttClient.publish("brink/FaultCode/get", String(fcode).c_str());

    mqttClient.publish("brink/OperationMsg/get", String(msg).c_str());

    mqttClient.publish("brink/I1/get", String(param1 - 100).c_str());
    
    mqttClient.publish("brink/U1/get", String(vstep1).c_str());
      
    mqttClient.publish("brink/U2/get", String(vstep2).c_str());
     
    mqttClient.publish("brink/U3/get", String(vstep3).c_str());
     
    mqttClient.publish("brink/U4/get", String(tU4/2).c_str());
     
    mqttClient.publish("brink/U5/get", String(tU5/2).c_str());
    
    mqttClient.publish("brink/Wifi/get", String(lRssi).c_str());

    mqttClient.publish("brink/OpenThermStatus/get", "WORK"); 
}

void loop()
{
  if (!mqttClient.connected()) 
  {
    MqttReconnect(); 
    mqttClient.publish("brink/OpenThermStatus/get", "WORK"); 
   
  }
  mqttClient.loop();
  
  currentTime = millis(); 
  if (currentTime - startTime >= readPeriod)  
  {
    ReadBrinkParameters();

    lRssi = WiFi.RSSI(); //long
    if ( abs(lRssi- lRssi_old) > 2) {   //reduce data publication due frequent slight changes of signal
      mqttClient.publish("brink/Wifi/get", String(lRssi).c_str());
      lRssi_old = lRssi;
    }
    startTime = currentTime;  
  }

}

//Reading selected Brink Renovent HR parameters, Mqtt publication only if a value has changed
void ReadBrinkParameters() 
{

  tIn  = ot.getVentSupplyInTemperature(); 
  if ( abs(tIn - tIn_old) > 0.2)  {  //reduce data publication due frequent slight changes of temp
      mqttClient.publish("brink/TempSupplyIn/get", String(tIn).c_str());
      tIn_old = tIn;
  }
 
  tOut = ot.getVentExhaustInTemperature();
  if ( abs(tOut - tOut_old) > 0.2) {   //reduce data publication due frequent slight changes of temp
      mqttClient.publish("brink/TempExhIn/get", String(tOut).c_str()); // String.toCharArray()
      tOut_old = tOut;
  }

  fault = ot.getFaultIndication();
  if (fault != fault_old) {
      mqttClient.publish("brink/FauiltIndication/get", String(fault).c_str()); 
      fault_old = fault;
  }
        
  vmode = ot.getVentilationMode();
  if (vmode != vmode_old) {
      mqttClient.publish("brink/VentilationMode/get", String(vmode).c_str());
      vmode_old = vmode;
  }
       
  sbypass = ot.getBrinkTSP(BypassStatus);         // ot.getBypassStatus() - this method does not work
  if (sbypass != sbypass_old) {
     mqttClient.publish("brink/BypassStaus/get", String(sbypass).c_str());
     sbypass_old = sbypass;
  }

/* does not work
  pbypass = ot.getBypassPosition();
  if (pbypass != pbypass_old) {
     mqttClient.publish("brink/BypassPosition/get", String(pbypass).c_str());
     pbypass_old = pbypass;
  }
*/

  filter = ot.getDiagnosticIndication();
  if (filter != filter_old) {
      mqttClient.publish("brink/FilterDirty/get", String(filter).c_str());
      filter_old = filter;
  }

  gvent =  ot.getVentilation();
  if (gvent != gvent_old) {
      mqttClient.publish("brink/VentilationNominalValue/get", String(gvent * 2.49).c_str());
      gvent_old = gvent;
  }

  pressin = ot.getBrinkTSP(CPID);
  if ( abs(pressin - pressin_old) > 1 ) { //reduce data publication due frequent slight changes of preassure
      mqttClient.publish("brink/CPID/get", String(pressin).c_str());
      pressin_old = pressin;
  }
 
  pressout = ot.getBrinkTSP(CPOD); 
  if ( abs(pressout - pressout_old) > 1)  {  //reduce data publication due frequent slight changes of preassure
      mqttClient.publish("brink/CPOD/get", String(pressout).c_str());
      pressout_old = pressout;
  }

  cvol = ot.getBrinkTSP(CurrentVol); 
  if (cvol != cvol_old) {
    mqttClient.publish("brink/CurrentVolume/get", String(cvol).c_str());
    cvol_old = cvol;
  }

  fcode = ot.getVentFaultCode();  
  if (fcode != fcode_old) {
      mqttClient.publish("brink/FaultCode/get", String(fcode).c_str());
      fcode_old = fcode;
  }

  vstep1 = ot.getBrinkTSP(U1);
  if (vstep1 != vstep1_old) {
      mqttClient.publish("brink/U1/get", String(vstep1).c_str());
      vstep1_old = vstep1;
  }

  vstep2 = ot.getBrinkTSP(U2);
  if (vstep2 != vstep2_old) {
      mqttClient.publish("brink/U2/get", String(vstep2).c_str());
      vstep2_old = vstep2;
  }

  vstep3 = ot.getBrinkTSP(U3);
  if (vstep3 != vstep3_old) {
      mqttClient.publish("brink/U3/get", String(vstep3).c_str());
      vstep3_old = vstep3;
  }

  tU4 = ot.getBrinkTSP(U4);
  if (tU4 != tU4_old) {
      mqttClient.publish("brink/U4/get", String(tU4/2).c_str());
      tU4_old = tU4;
  }

  tU5 = ot.getBrinkTSP(U5);
  if (tU5 != tU5_old) {
      mqttClient.publish("brink/U5/get", String(tU5/2).c_str());
      tU5_old = tU5;
  }

  msg = ot.getBrinkTSP(MsgOperation);
  if (msg != msg_old) {
      mqttClient.publish("brink/OperationMsg/get", String(msg).c_str());
      msg_old = msg;
  }

  param1 = ot.getBrinkTSP(I1);
  if (param1 != param1_old) {
      mqttClient.publish("brink/I1/get", String(param1 - 100).c_str());
      param1_old = param1;
  }

}
