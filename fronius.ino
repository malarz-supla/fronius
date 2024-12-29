/*
Copyright (C) AC SOFTWARE SP. Z O.O. & malarz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define STATUS_LED_GPIO 13
#define RELAY_GPIO 12
#define BUTTON_CFG_RELAY_GPIO 0

#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/control/action_trigger.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/network/html/custom_text_parameter.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/pv/fronius.h>

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;

Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, true); // inverted state
Supla::EspWebServer suplaServer;
#ifdef ARDUINO_ARCH_ESP32
#include <HTTPUpdateServer.h>
HTTPUpdateServer httpUpdater;
#else
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
#endif

// Those tags are used for HTML element names and for keys to access parameter
// values in Config storage class. Max length of those values is 15 chars.
const char FRONIUSIP[] = "FroniusIPAddr";

void setup() {
  
  Serial.begin(115200);

  // HTML www component (they appear in sections according to creation
  // sequence).
  new Supla::Html::DeviceInfo(&SuplaDevice);
  new Supla::Html::WifiParameters;
  new Supla::Html::ProtocolParameters;
  new Supla::Html::StatusLedParameters;
  
  // Here user defined inputs are defined.
  // Simple text input:
  // 15 - maximum text length accepted by your input
  new Supla::Html::CustomTextParameter(FRONIUSIP, "Adres IP inwertera Fronius", 15);

  Supla::Storage::Init();
  char ip[50] = {};
  if (Supla::Storage::ConfigInstance()->getString(FRONIUSIP, ip, 50)) {
    SUPLA_LOG_DEBUG(" +++++ Param[%s]: %s", FRONIUSIP, ip);
  } else {
    SUPLA_LOG_DEBUG(" +++++ Param[%s] is not set", FRONIUSIP);
  }
 
  // Channels configuration
  // CH 0 - Fronius inverter
  IPAddress ipAddr;
  ipAddr.fromString(ip);
  new Supla::PV::Fronius(ipAddr);

  // Buttons configuration
  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);
  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);

  SuplaDevice.setName("MALARZ Fronius Gate");

  // dodaj updater
  httpUpdater.setup(suplaServer.getServerPtr(), "/update");
  // configure defualt Supla CA certificate
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);
  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}
