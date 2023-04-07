#include <Arduino.h>
#include <OpenTherm.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// this part schall be changed ----------------------------------
const char* ssid = "your wifi"; 
const char* password = "your password";
const char* mqtt_server = "your mqtt server ip address"; 
const char* mqtt_client_id = "brink2"; // Unique client id
const int mqtt_Port = 1883; 
const char* mqtt_User = "your mqtt user"; 
const char* mqtt_Password = "your mqtt server password"; 
//float maxVent = 2.49; //it means 249 m/h3 - max avilable flow in my Brink - not used/needed
const unsigned long readPeriod = 1500; // 1000 = every second; set between 1000 - 5000
const unsigned long readPeriod_bypass = 120000; // Set +15000 - OT disconnection needed for bypass work
//-----------------------------------------------------------------------

const char* mqtt_topic_in = "brink/+/set"; //subscribe commands from Openhab
unsigned long startTime;
unsigned long currentTime;
unsigned long readOT;           //

const int HWCPin = 14; //Option: HW circulation pump D5
const int inPin = 4; //ESP8266 D2
const int outPin = 5; //ESP8266 D1 
OpenTherm ot(inPin, outPin);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float tIn, tIn_old = 0;         //temp external
float tOut, tOut_old = 0;       //temp internal
bool fault, fault_old = 1;      //fault code
bool vmode, vmode_old = 0;      //ventilation mode
bool bypass, bypass_old = 1;    //bypass mode
bool filter, filter_old = 1;    //filter replacement indicator

int pressin, pressin_old = 0;   //presure input duct [Pa]
int pressout, pressout_old = 0; //preasure output duct [Pa]
int vstep1, vstep1_old = 50;    //U1
int vstep2, vstep2_old = 150;   //U2
int vstep3, vstep3_old = 300;   //U3
int tU4, tU4_old = 1;           //U4 - atmospheric temp threshold for bypass
int tU5, tU5_old = 1;           //U5 - inside temp threshold for bypass
int cvol, cvol_old = 0;         //current ventilation capacity (out) [m/h3]
int RPMin, RPMin_old;           //RPM in - not used
int RPMout, RPMout_old;         //RPM out - not used
int fcode, fcode_old = 0;       //fault code
int msg, msg_old = 0;           //C-operation message
int param1, param1_old = 100;   //I1- imbalance parameter
long lRssi, lRssi_old = 0;      //Wifi signal level
bool sem_bypass;                //semaphore for bypass workaround


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

   // Setting/Changing selected Brink Renovent HR parameters requested by OpenHab
   if( strcmp(topic, "brink/VentNomValue/set") == 0 ) ot.setVentilation(atoi((char *)payload));  // uint8_t 
   if(strcmp(topic, "brink/U1/set") == 0) ot.setBrinkTSP(U1, atoi((char *)payload) );
   if(strcmp(topic, "brink/U2/set") == 0) ot.setBrinkTSP(U2, atoi((char *)payload) );
   if(strcmp(topic, "brink/U3/set") == 0) ot.setBrink2TSP(U3, atoi((char *)payload) );
   if(strcmp(topic, "brink/U4/set") == 0) ot.setBrinkTSP(U4, atoi((char *)payload)*2 );
   if(strcmp(topic, "brink/U5/set") == 0) {
      ot.setBrinkTSP(U5, atoi((char *)payload)*2 );
      delay(100);
      refreshAll(); // change of U5 triggers refresh of other paramteres
   }
   if(strcmp(topic, "brink/I1/set") == 0)ot.setBrinkTSP(I1, atoi((char *)payload)+ 100 );
    
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

    ReadBrinkParameters();
    refreshAll();
    sem_bypass = bypass;
    if (bypass == 1 ) readOT = readPeriod_bypass;
    else readOT = readPeriod;
    
    
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
 
    mqttClient.publish("brink/BypassStaus/get", String(bypass).c_str());
  
    mqttClient.publish("brink/FilterDirty/get", String(filter).c_str());
   
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

 //   mqttClient.publish("brink/OpenThermStatus/get", "WORK"); 
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
  if (currentTime - startTime >= readOT)  
  {
    ReadBrinkParameters(); 
    lRssi = WiFi.RSSI(); //long
    if ( abs(lRssi- lRssi_old) > 2) //reduce data publication due frequent slight changes of signal
    {   
      mqttClient.publish("brink/Wifi/get", String(lRssi).c_str());
      lRssi_old = lRssi;
    }
    
    // Workaround for bypass change and keeping change when U4 and U5 conditions are met
    if (sem_bypass == 0)  //bypass is CLOSED
    {
       if ( (tOut > tU5/2) && (tIn > tU4/2) && (tIn < tOut) ) // if true open bypass, 
       {
          mqttClient.publish("brink/OpenThermStatus/get", "WAIT"); // wait for bypass change
          delay(150000); //stop 2,5 min  
          readOT = readPeriod_bypass;
          sem_bypass = 1; 
       }
    }
    if (sem_bypass == 1) //bypass is OPEN
    {  
       if ( (tOut < tU5/2) || (tIn < tU4/2) || (tIn > tOut)  ) //if true close bypass
       {
          mqttClient.publish("brink/OpenThermStatus/get","WAIT"); // wait for bypass change
          delay(150000); //stop 2,5 min
          readOT = readPeriod;
          sem_bypass = 0; 
       }
    }
    startTime = currentTime;  
  }
  
}

//Reading all requested Brink HR parameters, Mqtt publication only if a value has changed
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
       
  bypass = ot.getBrinkTSP(BypassStatus);         // ot.getBypassStatus() - this method does not work
  if (bypass != bypass_old) {
     mqttClient.publish("brink/BypassStaus/get", String(bypass).c_str());
     bypass_old = bypass;
  }

  filter = ot.getDiagnosticIndication();
  if (filter != filter_old) {
      mqttClient.publish("brink/FilterDirty/get", String(filter).c_str());
      filter_old = filter;
  }

  pressin = ot.getBrink2TSP(CPID);
  if ( abs(pressin - pressin_old) > 1 ) { //reduce data publication due frequent slight changes of preassure
      mqttClient.publish("brink/CPID/get", String(pressin).c_str());
      pressin_old = pressin;
  }
 
  pressout = ot.getBrink2TSP(CPOD); 
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

  vstep3 = ot.getBrink2TSP(U3);
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
   mqttClient.publish("brink/OpenThermStatus/get", "WORK"); //means OT is connected
}
