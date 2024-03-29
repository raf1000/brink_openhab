/*
OpenTherm.h - OpenTherm Library for the ESP8266/Arduino platform
https://github.com/ihormelnyk/OpenTherm
http://ihormelnyk.com/pages/OpenTherm
Licensed under MIT license
Copyright 2018, Ihor Melnyk
Updated by raf1000 in relation to "Brink Renovent HR"

Frame Structure:
P MGS-TYPE SPARE DATA-ID  DATA-VALUE
0 000      0000  00000000 00000000 00000000
*/

#ifndef OpenTherm_h
#define OpenTherm_h

#include <stdint.h>
#include <Arduino.h>

enum OpenThermResponseStatus {
	NONE,
	SUCCESS,
	INVALID,
	TIMEOUT
};


enum OpenThermMessageType {
	/*  Master to Slave */
	READ_DATA       = B000,
	READ            = READ_DATA, // for backwared compatibility
	WRITE_DATA      = B001,
	WRITE           = WRITE_DATA, // for backwared compatibility
	INVALID_DATA    = B010,
	RESERVED        = B011,
	/* Slave to Master */
	READ_ACK        = B100,
	WRITE_ACK       = B101,
	DATA_INVALID    = B110,
	UNKNOWN_DATA_ID = B111
};

typedef OpenThermMessageType OpenThermRequestType; // for backwared compatibility

enum OpenThermMessageID {
	Status, // flag8 / flag8  Master and Slave Status flags.
	TSet, // f8.8  Control setpoint  ie CH  water temperature setpoint (°C)
	MConfigMMemberIDcode, // flag8 / u8  Master Configuration Flags /  Master MemberID Code
	SConfigSMemberIDcode, // flag8 / u8  Slave Configuration Flags /  Slave MemberID Code
	Command, // u8 / u8  Remote Command
	ASFflags, // / OEM-fault-code  flag8 / u8  Application-specific fault flags and OEM fault code
	RBPflags, // flag8 / flag8  Remote boiler parameter transfer-enable & read/write flags
	CoolingControl, // f8.8  Cooling control signal (%)
	TsetCH2, // f8.8  Control setpoint for 2e CH circuit (°C)
	TrOverride, // f8.8  Remote override room setpoint
	TSP, // u8 / u8  Number of Transparent-Slave-Parameters supported by slave
	TSPindexTSPvalue, // u8 / u8  Index number / Value of referred-to transparent slave parameter.
	FHBsize, // u8 / u8  Size of Fault-History-Buffer supported by slave
	FHBindexFHBvalue, // u8 / u8  Index number / Value of referred-to fault-history buffer entry.
	MaxRelModLevelSetting, // f8.8  Maximum relative modulation level setting (%)
	MaxCapacityMinModLevel, // u8 / u8  Maximum boiler capacity (kW) / Minimum boiler modulation level(%)
	TrSet, // f8.8  Room Setpoint (°C)
	RelModLevel, // f8.8  Relative Modulation Level (%)
	CHPressure, // f8.8  Water pressure in CH circuit  (bar)
	DHWFlowRate, // f8.8  Water flow rate in DHW circuit. (litres/minute)
	DayTime, // special / u8  Day of Week and Time of Day
	Date, // u8 / u8  Calendar date
	Year, // u16  Calendar year
	TrSetCH2, // f8.8  Room Setpoint for 2nd CH circuit (°C)
	Tr, // f8.8  Room temperature (°C)
	Tboiler, // f8.8  Boiler flow water temperature (°C)
	Tdhw, // f8.8  DHW temperature (°C)
	Toutside, // f8.8  Outside temperature (°C)
	Tret, // f8.8  Return water temperature (°C)
	Tstorage, // f8.8  Solar storage temperature (°C)
	Tcollector, // f8.8  Solar collector temperature (°C)
	TflowCH2, // f8.8  Flow water temperature CH2 circuit (°C)
	Tdhw2, // f8.8  Domestic hot water temperature 2 (°C)
	Texhaust, // s16  Boiler exhaust temperature (°C)
	TdhwSetUBTdhwSetLB = 48, // s8 / s8  DHW setpoint upper & lower bounds for adjustment  (°C)
	MaxTSetUBMaxTSetLB, // s8 / s8  Max CH water setpoint upper & lower bounds for adjustment  (°C)
	HcratioUBHcratioLB, // s8 / s8  OTC heat curve ratio upper & lower bounds for adjustment
	TdhwSet = 56, // f8.8  DHW setpoint (°C)    (Remote parameter 1)
	MaxTSet, // f8.8  Max CH water setpoint (°C)  (Remote parameters 2)
	Hcratio, // f8.8  OTC heat curve ratio (°C)  (Remote parameter 3)

// Ventilation Specific Message IDs
	VentStatus = 70, // flag8 / flag8  Master and Slave Status flags specific to Vent systems, READ STATUS V/H , HB0-HB3,LB0-LB5
	VentNomVentSet, // _ / u8 Set the relative ventilation 0-100%, WRITE, CONTROL SETPOINT V/H
	VentFault, // flag8 / flag8 Application specific fault flags, READ (u8) FAULT FLAGS/CODE V/H
	VentOEMFault, // flag8 / flag8 Oem specific fault flags, DIAGNOSTIC CODE V/H,READ,U16
	VentSlaveVentConfig = 74, // Slave Configuration Flags /  Slave MemberID Code, CONFIG/MEMBERID V/H,READ,U8  , HB0-HB2, LB
	VentOTversion,  // f8.8 OPENTHERM VERSION V/H,READ, 0,127,“2,32”,Yes
        VentVersionType,  // VERSION & TYPE V/H,READ,U8,0,255,1,U8,0,255,0,Yes
	VentNomVent = 77, // _ / u8 Read the relative ventilation 0-100%, RELATIVE VENTILATION,READ,U8,0,255
	VentRelHumid, // _ / u8 Read the relative humidity 0-100%, RELATIVE HUMIDITY,READ,U8,0,255, RELATIVE HUMIDITY,WRITE,U8,0,255
	VentCO2level, // u16 CO2 level in PPM (0-2000), CO2 LEVEL,READ,U16,0,10000  , CO2 LEVEL,WRITE,U16,0,10000
	VentTsupplyin, // f8.8 Supply Outlet temperature  SUPPLY INLET TEMPERATURE,READ,F8.8,0,127,“0,00”
	VentTsupplyout, // f8.8 Supply Outlet temperature  SUPPLY OUTLET TEMPERATURE,READ,F8.8,0,127,“0,00”
	VentTexhaustin,// f8.8 Exhaust Inlet temperature EXHAUST INLET TEMPERATURE,READ,F8.8,0,127,“0,00”
	VentTexhaustout, // f8.8 Exhaust Outlet temperature  EXHAUST OUTLET TEMPERATURE,READ,F8.8,0,127,“0,00"
	VentRPMexhaust, // u16 Actual RPM for inlet fan ACTUAL EXHAUST FAN SPEED,READ,U16,0,10000
	VentRPMsupply, // u16 Actual RPM for supply fan ACTUAL INLET FAN SPEED”,READ,U16,0,10000
	VentRemoteParam,  //  REMOTE PARAMETER SETTINGS V/H,READ,FLAG,  HBO, LBO
	VentNomRelVent = 87, // _ / u8 Get the current relative ventilation NOMINAL VENTIALTION VALUE,READ,U8,0,255,  NOMINAL VENTIALTION VALUE,WRITE,U8,0,255
	VentTSPNumber, // TSP NUMBER V/H,READ,U8,0,255,0,U8
	VentTSPEntry,  // TSP ENTRY V/H,READ,U8,0,255,0,U8,0,255   TSP ENTRY V/H,WRITE,U8,0,255,0,U8,0,255
	VentFaultBufferSize,  // FAULT BUFFER SIZE V/H ,READ,U8,0,255,0,U8,0,0,
	VentFaultBufferEntry,  // FAULT BUFFER ENTRY V/H READ,U8,0,255,0,U8,0,255

	RemoteOverrideFunction = 100, // flag8 / -  Function of manual and program changes in master and remote room setpoint.
	OEMDiagnosticCode = 115, // u16  OEM-specific diagnostic/service code
	BurnerStarts, // u16  Number of starts burner
	CHPumpStarts, // u16  Number of starts CH pump
	DHWPumpValveStarts, // u16  Number of starts DHW pump/valve
	DHWBurnerStarts, // u16  Number of starts burner during DHW mode
	BurnerOperationHours, // u16  Number of hours that burner is in operation (i.e. flame on)
	CHPumpOperationHours, // u16  Number of hours that CH pump has been running
	DHWPumpValveOperationHours, // u16  Number of hours that DHW pump has been running or DHW valve has been opened
	DHWBurnerOperationHours, // u16  Number of hours that burner is in operation during DHW mode
	OpenThermVersionMaster, // f8.8  The implemented version of the OpenTherm Protocol Specification in the master.
	OpenThermVersionSlave, // f8.8  The implemented version of the OpenTherm Protocol Specification in the slave.
	MasterVersion, // u8 / u8  Master product version number and type
	SlaveVersion, // u8 / u8  Slave product version number and type
};

enum OpenThermStatus {
	NOT_INITIALIZED,
	READY,
	DELAY,
	REQUEST_SENDING,
	RESPONSE_WAITING,
	RESPONSE_START_BIT,
	RESPONSE_RECEIVING,
	RESPONSE_READY,
	RESPONSE_INVALID
};

// Brink Renovent HR specific codes
enum BrinkTSPindex {
	U1, 	// Volume Step 1 - 2 bytes
	U2 = 2, // Volume Step 2 - 2 byts 
	U3 = 4,	// Volume Step 3 - 2 bytes
	U4 = 6, // Minimum atmospheric temperature bypass (value multiplied *2, ex 15C <=> 30)
	U5 = 7, // Minimum indor temperature bypass (value multiplied *2)
	U6,	// option PCB: postheater temp seting, control postheater up to 1000 W
	U7,	// option PCB: proportional input mode (ex moisture sensor, CO2 sensor): A = only 3-way switch, b = proportional input 1, C = proportional input 2, d = proportional input 1 and proportional input 2 (input 1 precedes input 2)	
	U8,  	// Not applicable
	I1,     // Fixed imbalance (value shifted by 100, ex 100 <=> 0)
	I2,	// No contact step			
	I3,	// not applicable
	I4,	// Switch line step 1
	I5,	// Switch line step 2
	I6,	// Switch line step 3
	I7,	// Imbalance permissible
	I8,	// Bypass mode
	I9,	// Hysteresis bypass
	I10,	// Constand pressure switched off
	I11,	// Preheater (0-3), o = no preheater
   	I12,    // Offset temperature preheater(value shifted by 60, 60 <=> 0, 61 <=> 0.5)
	I13,	// Filter message on/off (1=on)
	I14,	// Option PCB present (0 = no)
	I15,	// Heat recovery configuration (0=heat recovery, 1 = central heating + heat recovery)
	I16,	// Fan setting for central heating + heat recovery
	I17,	// Repeat time in hours for switching off the fan(s) selected under I16
	I18,	// Maximum switch-off time in seconds for the fan(s) selected under I16
	I19,	// Minimum switch-off time in seconds for the fan(s) selected under I16 
	
	P1 = 31,// option PCB: air volume of the supply volume in case of emergency
	P2, 	// option PCB: air volume of the exhaust fan in case of emergency 
	P3, 	// option PCB: additional air volume supply fan when bedroom valve entrance is closed (this volume is added to the volume related to 3 way switch)
	P4, 	// option PCB: additional air volume exhaust fan when bedroom valve entrance is closed
	P5, 	// option PCB: programmable make contact 1, indicates where make contact 1 should be connected, 0 = not linked, 1 = overrule frost control, 2 = link to bypass conditions, 3 = link to frost conditions, 4 = bypass valve control
	P6, 	// option PCB: programmable make contact 1, determines how the supply fan reacts when make contact 1 is made: 0 = volume to absolute minimum, 1 = volume according to 3-way switch setting, 2 = volume according to position 3 of the 3-way switch, 3 = fan off
	P7, 	// option PCB: programmable make contact 1, determines how the exhaust fan reacts when make contact 1 is made: 0 = volume to absolute minimum, 1 = volume according to 3-way switch setting, 2 = volume according to position 3 of the 3-way switch, 3 = fan off
	P8, 	// option PCB: programmable make contact 2, indicates where make contact 1 should be connected, 0 = not linked, 1 = overrule frost control, 2 = link to bypass conditions, 3 = link to frost conditions, 4 = bypass valve control
	P9, 	// option PCB: programmable make contact 2, determines how the supply fan reacts when make contact 1 is made: 0 = volume to absolute minimum, 1 = volume according to 3-way switch setting, 2 = volume according to position 3 of the 3-way switch, 3 = fan off
	P10, 	// option PCB: programmable make contact 2, determines how the exhaust fan reacts when make contact 1 is made: 0 = volume to absolute minimum, 1 = volume according to 3-way switch setting, 2 = volume according to position 3 of the 3-way switch, 3 = fan off
	P11,    // option PCB: determines the target voltage of the proportional input 1 (moisture sensor) 0-10V . The volume control tries to adjust the input voltage to the target voltage within the set conditions.
	P12,    // option PCB: gives the maximum voltage of the device connected to the proportional input 1 (moisture sensor),tThe proportional band of the PI controller is automatically adjusted.
	P13, 	// option PCB: defines the integration time of the PI controller from the proportional input 1 (moisture sensor). The PI controller controls pure proportional if the integration time is 0 second.	
	P14,  	// option PCB: determines the target voltage of the proportional input 2 (CO2 sensor) 0-10V . The volume control tries to adjust the input voltage to the target voltage within the set conditions.
	P15,  	// option PCB: gives the maximum voltage of the device connected to the proportional input 2 (CO2 sensor),tThe proportional band of the PI controller is automatically adjusted.
	P16, 	// option PCB: defines the integration time of the PI controller from the proportional input 2 (Co2 sensor). The PI controller controls pure proportional if the integration time is 0 second.	
	P17,    // option PCB: control preheater up to 1000 W, 0 = no preheater, 1 = preheater present
	
	MaxVol,	//  Maximum avilable volume [m3/h] - in/out - 2 bytes
	MinVol = 50, //Minimum allowed volume (50m3/h for HR Large) - 2 bytes
	CurrentVol = 52, // Current position/outlet volume [m3/h]
	MsgOperation, //?Message code operating conditions: C0 = no messages, C3 = input fan runs in constant pressure, C6 = output fan runs in constand pressure, C7 = correction max air flaw, C8 = imbalance, C12 = emergency mode
	BypassStatus, // Bypass status 0=bypass valve shut, 1 = bypass valve automatic, 2 = inpute at minimum
	TempAtmo,  // Temperature from atmosphere [°C] (shifted by 100) 
	TempIndoors,  // Temperature from indors [°C] (shifted by 100) 
	InitStatus,  // 1 = aplliance initiated, 0 = not initiated
	VoltageParam1,	// option PCB: voltage on proportional input 1 (moisture sensor) 0-10V
	VoltageParam2,	// option PCB: voltage on proportional input 2 (CO2 sensor) 0-10V
	CurrentInputVol, //Current input volume [m3/h] - 2 bytes
	CurrentOutputVol = 62, //Current output volume [m3/h] - 2 bytes
	CPID = 64,	//  Current pressure input duct [Pa]  - 2 bytes
	CPOD = 66,  //  Current pressure output duct [Pa] - 2 bytes
	FrostStatus = 68,// ?Status frost protection: 0 = none, 1-4 = imbalance, 5 = input fan off
	Temp2Atmo, // ?Temperature to atmosphere, sensor not connected as standard [°C] (shifted by 100) 
	Temp2Indoors, // ?Temperature to indoors, sensor not connected as standard [°C] (shifted by 100) 
	TempPostHeater, // ?Temperature postheater 0 = 0°C (not active)
	
};



class OpenTherm
{
public:
	OpenTherm(int inPin = 4, int outPin = 5, bool isSlave = false);
	volatile OpenThermStatus status; //zdefiniowanie zminnej wspoldzielonej z ICHACHE
	void begin(void(*handleInterruptCallback)(void));
	void begin(void(*handleInterruptCallback)(void), void(*processResponseCallback)(unsigned long, OpenThermResponseStatus));
	bool isReady();
	unsigned long sendRequest(unsigned long request);
	bool sendResponse(unsigned long request);
	bool sendRequestAync(unsigned long request);
	unsigned long buildRequest(OpenThermMessageType type, OpenThermMessageID id, unsigned int data);
	unsigned long buildResponse(OpenThermMessageType type, OpenThermMessageID id, unsigned int data);
	OpenThermResponseStatus getLastResponseStatus();
	const char *statusToString(OpenThermResponseStatus status);
	void handleInterrupt();
	void process();
	void end();

	bool parity(unsigned long frame);
	OpenThermMessageType getMessageType(unsigned long message);
	OpenThermMessageID getDataID(unsigned long frame);
	const char *messageTypeToString(OpenThermMessageType message_type);
	bool isValidRequest(unsigned long request);
	bool isValidResponse(unsigned long response);
	

	//requests
	unsigned long buildSetBoilerStatusRequest(bool enableCentralHeating, bool enableHotWater = false, bool enableCooling = false, bool enableOutsideTemperatureCompensation = false, bool enableCentralHeating2 = false);
	unsigned long buildSetBoilerTemperatureRequest(float temperature);
	unsigned long buildGetBoilerTemperatureRequest();

	//responses
	bool isFault(unsigned long response);
	bool isCentralHeatingActive(unsigned long response);
	bool isHotWaterActive(unsigned long response);
	bool isFlameOn(unsigned long response);
	bool isCoolingActive(unsigned long response);
	bool isDiagnostic(unsigned long response);
	uint16_t getUInt(const unsigned long response) const;
	float getFloat(const unsigned long response) const;	
	unsigned int temperatureToData(float temperature);
	// Brink renovent HR
	uint8_t getU8 (const unsigned long response) const; 

	//basic requests
	unsigned long setBoilerStatus(bool enableCentralHeating, bool enableHotWater = false, bool enableCooling = false, bool enableOutsideTemperatureCompensation = false, bool enableCentralHeating2 = false);
	bool setBoilerTemperature(float temperature);
	float getBoilerTemperature();
   	float getReturnTemperature();
	
    	bool setDHWSetpoint(float temperature);
    	float getDHWTemperature();
    	float getModulation();
    	float getPressure();
    	unsigned char getFault();

	
	//Ventilation systems requests
	unsigned int getVentilation();
	unsigned int setVentilation(unsigned int nominal_value);
	float getVentSupplyInTemperature();
	float getVentSupplyOutTemperature();
	float getVentExhaustInTemperature();
	float getVentExhaustOutTemperature();

//Brink Renovent HR
	bool getFaultIndication();
	bool getVentilationMode();
	bool getBypassStatus();  // does not work
	bool getBypassAutomaticStatus(); // does not work
	bool getDiagnosticIndication();

	bool getBypassPosition(); // does not work
	bool getBypassMode(); // does not work 
	
//Brink Renovent HR
	uint8_t getBrinkTSP(BrinkTSPindex index) ;
	bool setBrinkTSP(BrinkTSPindex index, uint8_t value);
	unsigned int getVentRPM(OpenThermMessageID id);
	uint8_t getVentFaultCode();
	bool setBrink2TSP(BrinkTSPindex first_index, uint16_t value); 
	uint16_t getBrink2TSP(BrinkTSPindex first_index);

private:
	const int inPin;
	const int outPin;
	const bool isSlave;

	volatile unsigned long response;
	volatile OpenThermResponseStatus responseStatus;
	volatile unsigned long responseTimestamp;
	volatile byte responseBitIndex;

	int readState();
	void setActiveState();
	void setIdleState();
	void activateBoiler();

	void sendBit(bool high);
	void(*handleInterruptCallback)();
	void(*processResponseCallback)(unsigned long, OpenThermResponseStatus);
};

#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#endif

#endif // OpenTherm_h
