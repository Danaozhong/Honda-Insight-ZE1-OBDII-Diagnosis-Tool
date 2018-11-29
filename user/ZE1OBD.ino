// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Sketch shows how to use SimpleBLE to advertise the name of the device and change it on the press of a button
// Useful if you want to advertise some sort of message
// Button is attached between GPIO 0 and GND, and the device name changes each time the button is pressed

#include "app/diagnosis_reader_application.hpp"

#include "driver/ble/ble_interface.h"


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//OBDIICCBluetooth ble;

Application::DiagnosisReaderApplication* application;

void onButton(){
	String out = "";
	out += "TestBLEInterface";
	Serial.println(out);
	//ble.begin(out);
}

void setup() 
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	pinMode(0, INPUT_PULLUP);
	Serial.print("ESP32 SDK: ");
	Serial.println(ESP.getSdkVersion());
	//ble.begin("ESP32 SimpleBLE");
	//Serial.println("Press the button to change the device's name");

	application = new Application::DiagnosisReaderApplication();
}

extern int diagnosis_tool_main(void);

void loop() 
{
	application->loop();
	while (Serial.available()) Serial.write(Serial.read());
}
