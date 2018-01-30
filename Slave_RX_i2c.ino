// Wire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 1/25/2018

#include <Wire.h>
#include <String.h>

#define RELAY_PIN 3

#define MAX_BYTES_RECEIVED 3 //we only are sending to turn ON, OFF, STATUS
// I think its 3 bytes because, you first send address
// then you send register you want to talk to
// then you send the value you want to send. 

#define REGISTER_MAP_SIZE   3// ADDRESS, STATUS, ON
#define SLAVE_ADDRESS   1 //whats a good way to choose?


//Address Map for COMMANDS
#define TURN_ON_REG			(0X01)
#define	STATUS_REG			(0X02)


//Control Flags
int update_register = 0;
int relay_state = 0; //default off;

int new_address; //latest address for the slave.

byte registerMap[REGISTER_MAP_SIZE];
byte receievedCommands[MAX_BYTES_RECEIVED];


//IRS prototypes
void requestEvent(void);
void receiveEvent(void);

void setup() {
  Wire.begin(SLAVE_ADDRESS);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Wire.onRequest(requestEvent); // register interrupt requestEvent, when the master asks for STATUS
    Wire.onRequest(requestEvent);


  
  Serial.begin(9600);           // start serial for output
  Serial.println("Slave awake");

  //led set up led indirectly hooked up to pin 3 right now.
  // pin 3 high is led on
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(RELAY_PIN, LOW);
  
  registerMap[1] = 0;
  
  new_address = SLAVE_ADDRESS;
  
}

void loop() {
	Serial.print("the address of the slave is:  ");
	Serial.println(new_address);
	//check the flags ... polling?
	if(update_register == 1){
		//update the stuff
		update(); // updates the relay state only. 
		//TODO: add ability to change slave address. 
		update_register = 0; //reset flag`
	}
	
	//check here the state of the relay, in registger map
	// set relay accordingly. 
	if(registerMap[1] == 1){
		
		//update status register
		registerMap[2] = "test";
		
		digitalWrite(RELAY_PIN, HIGH);
		digitalWrite(13, HIGH);
	}
	if(registerMap[1] == 0){
		
		//update status register

		digitalWrite(RELAY_PIN, LOW);
		digitalWrite(13, LOW);
	}
  delay(100);
}


void update(){
	//write to the memory register. status register?
	registerMap[1] = relay_state;
}



//When the slave receives data from the master
//we know we expect only the address and a single command.
// so we know we only are expecting 2 bytes. 
void receiveEvent(int bytesReceived) {	
  for(int i = 0; i < bytesReceived; i++){
	  //loop through the data from the master
	  if(i < MAX_BYTES_RECEIVED){
		  
		  receievedCommands[i] = Wire.read();
	  }
	  else{
		Wire.read(); // let them come but don't collect
	  }
  }
	  
	  Serial.print("index 0: ");
	  Serial.println(receievedCommands[0]);
	  
	  Serial.print("index 1: ");
	  Serial.println(receievedCommands[1]);
//now we have collected info, now we need to parse it. 
// so we are really parsing the command here, and 
// use a switch statement to change based on the command
// this provides flexibility just in case
// we change the meaning of a given address.
//this is how we filter out a read vs a write register.

switch(receievedCommands[0]){
	//case change the slave's address
	case 0x00:
	update_register = 1;
	new_address = receievedCommands[1];
	Serial.println("****************/n/n");
		Wire.begin(new_address);
		Serial.println(new_address, HEX);
	Serial.println("****************/n/n");
		bytesReceived--; 
		if(bytesReceived == 1){
			return; // only expecting 2 bytes 
		}

	//case TURN_ON_REG:
	case 0x01:
	//next byte is the state, 1 is on, 0 is off. 

	relay_state = receievedCommands[1]; 
	update_register = 1;
		bytesReceived--; 
		if(bytesReceived == 1){
			return; // only expecting 2 bytes 
		}
		
		

	default:
	//trying to write to a READ-ONLY register.
	
	digitalWrite(13, LOW);
		return;// out of bounds
	}
	  

}// end of receive ISR

//When the slave receives data from the bus 
// Wire.requestFrom(SLAVE_ADDRESS, REGISTER_MAP_SIZE)
void requestEvent() {
	
	Wire.write(registerMap, REGISTER_MAP_SIZE);
	
	//String statusRegisterString = String(registerMap[2]);
	//Serial.print("Status   ");
	//Serial.println(registerMap[2]);
	
	
	//Wire.write(1);
	//Wire.write(64);
	//Wire.write(97);
	
	
	//we will send entire map, but we only need to 
	// send the status, so probably bit shift?
	
}// end of request ISR
