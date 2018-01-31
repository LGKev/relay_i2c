// Wire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 1/25/2018

#include <Wire.h>
#include <String.h>

//#define DEBUG_OUTPUT


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
volatile bool update_register = false;
volatile bool relay_state = false; //default off;

volatile byte new_address; //latest address for the slave.

byte registerMap[REGISTER_MAP_SIZE];
volatile byte receievedCommands[MAX_BYTES_RECEIVED];


//ISR prototypes
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
#ifdef DEBUG_OUTPUT
	Serial.print("the RxCommand[0] @main loop top is:   ");
	Serial.println(receievedCommands[0]);
	Serial.print("the address of the slave is:  ");
	Serial.println(new_address);
#endif
  relayConfig();

  //check the flags ... polling?
  if (update_register == 1) {
    //update the stuff
    update(); // updates the relay state only.
    update_register = 0; //reset flag`
  }
  //check here the state of the relay, in register map
  // set relay accordingly.
  if (registerMap[1] == 1) {
	  //TODO: add the status register update here.
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(13, HIGH);
  }
  if (registerMap[1] == 0) {
		//TODO: add the status register update here
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(13, LOW);
  }
}

/*========================================================*/
// 				Helper Functions
/*========================================================*/


/*
		@brief: Parses the commands received to determine how the
			slave device should behave. this is where the address 
			changes, and where the state of the relay changes
			DOES NOT update the actual relay output here, that is done in
			the main loop.
		@input: *global* receivedCommands
		@returns: nothing
		
		@flags:  update_register flag to 1
*/
void relayConfig() {
  //now we have collected info, now we need to parse it.
  // so we are really parsing the command here, and
  // use a switch statement to change based on the command
#ifdef DEBUG_OUTPUT
Serial.print("the RxCommand[0] is:   ");
Serial.println(receievedCommands[0]);
#endif

  switch (receievedCommands[0]) {
    case 0x03: //change slave address
      update_register = 1;
      new_address = receievedCommands[1];
 #ifdef DEBUG_OUTPUT
	Serial.print("121 the newest address is: ");
	Serial.println(new_address);
	  for(int i=0; i< 10; i++){
      digitalWrite(13, HIGH);
      delay(75);
      digitalWrite(13, LOW);
      delay(75); 
      }
 #endif
//      bytesReceived--;
//      if (bytesReceived == 1) {
//        return; // only expecting 2 bytes
//      }
    break;
    case 0x01: //change relay state: on or off
      //next byte is the state, 1 is on, 0 is off.
      relay_state = receievedCommands[1];
      update_register = 1;
//      bytesReceived--;
//      if (bytesReceived == 1) {
//        return; // only expecting 2 bytes
//      }
    break;
    default:
      //trying to write to a READ-ONLY register.
      digitalWrite(13, LOW);
      return;// out of bounds
  }
}

/*
		@brief: 
		@input: 
		@returns: nothing
		
		@flags:  update_register flag to 1
*/
void update() {
  //write to the memory register. status register?
  Wire.begin(new_address);
  registerMap[1] = relay_state;
}

//When the master requests data from slave
void receiveEvent(int bytesReceived) {
  for (int i = 0; i < bytesReceived; i++) {
    //loop through the data from the master
    if (i < MAX_BYTES_RECEIVED) {
      receievedCommands[i] = Wire.read(); //all commands and data are collected in the ISR... do not process here.
    }
    else {
      Wire.read(); // let them come but don't collect
    }
  }
}// end of receive ISR

//When the slave receives data from the bus
// Wire.requestFrom(SLAVE_ADDRESS, REGISTER_MAP_SIZE)
void requestEvent() {
  Wire.write(registerMap, REGISTER_MAP_SIZE);
  //we will send entire map, but we only need to
  // send the status, so probably bit shift?
}// end of request ISR
