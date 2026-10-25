
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
// Konfiguracja



const char* COMMAND_TOPIC = "devices/esp32-001/commands";
const char* RESPONSE_TOPIC = "devices/esp32-001/responses";

constexpr int LED_PIN = 2;

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

void onMessage(char* topic, byte* payload, unsigned int length) {
    JsonDocument command;
    DeserializationError error =
        deserializeJson(command, payload, length);

    if (error) {
        Serial.println("Niepoprawny JSON");
        return;
    }

    String action = command["action"] | "";
    String commandId = command["command_id"] | "";

    bool success = true;

    if (action == "led_on") {
        digitalWrite(LED_PIN, HIGH);
    } else if (action == "led_off") {
        digitalWrite(LED_PIN, LOW);
    } else if (action != "get_led_state") {
        success = false;
    }

    JsonDocument response;
    response["command_id"] = commandId;
    response["success"] = success;
    response["led"] =
        digitalRead(LED_PIN) == HIGH ? "on" : "off";

    String output;
    serializeJson(response, output);

    mqtt.publish(RESPONSE_TOPIC, output.c_str());
    Serial.println(output);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Laczenie Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi polaczone");

    // Synchronizacja czasu dla TLS
    configTime(0, 0, "pool.ntp.org");

    while (time(nullptr) < 1700000000) {
        delay(500);
    }

    secureClient.setCACert(ROOT_CA);

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMessage);
    mqtt.setBufferSize(512);

    Serial.println("Konfiguracja MQTT gotowa");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        return;
    }

    if (!mqtt.connected()) {
        Serial.println("Laczenie MQTT...");

        if (mqtt.connect(
                "esp32-001",
                MQTT_USER,
                MQTT_PASS
            )) {
            mqtt.subscribe(COMMAND_TOPIC);
            Serial.println("MQTT polaczone!");
        } else {
            Serial.print("MQTT error: ");
            Serial.println(mqtt.state());
            delay(5000);
        }
    }

    mqtt.loop();
}
