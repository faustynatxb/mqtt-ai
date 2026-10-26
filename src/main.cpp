#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

const char* COMMAND_TOPIC = "devices/esp32-001/commands";
const char* RESPONSE_TOPIC = "devices/esp32-001/responses";

constexpr int LED_PIN = 2;
constexpr int OUTPUT_PINS[] = {2, 4, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
constexpr int INPUT_PINS[] = {32, 33, 34, 35, 36, 39};
constexpr int ANALOG_INPUT_PINS[] = {32, 33, 34, 35, 36, 39};
constexpr size_t PWM_CHANNEL_COUNT = 8;
constexpr size_t MAX_TIMED_OUTPUTS = 8;
constexpr uint32_t PWM_FREQUENCY = 5000;
constexpr uint8_t PWM_RESOLUTION_BITS = 8;
constexpr int PWM_MAX_DUTY = (1 << PWM_RESOLUTION_BITS) - 1;
constexpr int MAX_DURATION_SECONDS = 3600;
constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 5000;

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

int pwmPins[PWM_CHANNEL_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1};

enum TimedOutputType {
    TIMED_DIGITAL,
    TIMED_PWM,
};

struct TimedOutput {
    bool active = false;
    int pin = -1;
    TimedOutputType type = TIMED_DIGITAL;
    uint32_t stopAt = 0;
};

TimedOutput timedOutputs[MAX_TIMED_OUTPUTS];
uint32_t lastMqttConnectAttempt = 0;

bool containsPin(const int* pins, size_t count, int pin) {
    for (size_t index = 0; index < count; index++) {
        if (pins[index] == pin) {
            return true;
        }
    }
    return false;
}

bool isAllowedOutputPin(int pin) {
    return containsPin(OUTPUT_PINS, sizeof(OUTPUT_PINS) / sizeof(OUTPUT_PINS[0]), pin);
}

bool isAllowedInputPin(int pin) {
    return containsPin(INPUT_PINS, sizeof(INPUT_PINS) / sizeof(INPUT_PINS[0]), pin);
}

bool isAllowedAnalogInputPin(int pin) {
    return containsPin(
        ANALOG_INPUT_PINS,
        sizeof(ANALOG_INPUT_PINS) / sizeof(ANALOG_INPUT_PINS[0]),
        pin
    );
}

int pwmChannelForPin(int pin) {
    for (size_t channel = 0; channel < PWM_CHANNEL_COUNT; channel++) {
        if (pwmPins[channel] == pin) {
            return static_cast<int>(channel);
        }
    }
    return -1;
}

int attachPwm(int pin) {
    int existingChannel = pwmChannelForPin(pin);
    if (existingChannel >= 0) {
        return existingChannel;
    }

    for (size_t channel = 0; channel < PWM_CHANNEL_COUNT; channel++) {
        if (pwmPins[channel] == -1) {
            ledcSetup(channel, PWM_FREQUENCY, PWM_RESOLUTION_BITS);
            ledcAttachPin(pin, channel);
            pwmPins[channel] = pin;
            return static_cast<int>(channel);
        }
    }

    return -1;
}

void stopPwm(int pin) {
    int channel = pwmChannelForPin(pin);
    if (channel < 0) {
        return;
    }

    ledcWrite(channel, 0);
    ledcDetachPin(pin);
    pwmPins[channel] = -1;
}

void clearTimedOutput(int pin) {
    for (TimedOutput& output : timedOutputs) {
        if (output.active && output.pin == pin) {
            output.active = false;
        }
    }
}

bool canScheduleTimedOutput(int pin) {
    for (const TimedOutput& output : timedOutputs) {
        if (!output.active || output.pin == pin) {
            return true;
        }
    }
    return false;
}

bool scheduleTimedOutput(int pin, TimedOutputType type, int durationSeconds) {
    clearTimedOutput(pin);

    if (durationSeconds == 0) {
        return true;
    }

    for (TimedOutput& output : timedOutputs) {
        if (!output.active) {
            output.active = true;
            output.pin = pin;
            output.type = type;
            output.stopAt = millis() + static_cast<uint32_t>(durationSeconds) * 1000UL;
            return true;
        }
    }

    return false;
}

void serviceTimedOutputs() {
    uint32_t now = millis();

    for (TimedOutput& output : timedOutputs) {
        if (!output.active || static_cast<int32_t>(now - output.stopAt) < 0) {
            continue;
        }

        if (output.type == TIMED_DIGITAL) {
            stopPwm(output.pin);
            pinMode(output.pin, OUTPUT);
            digitalWrite(output.pin, LOW);
        } else {
            stopPwm(output.pin);
        }

        output.active = false;
    }
}

bool setDigitalOutput(int pin, bool high) {
    stopPwm(pin);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, high ? HIGH : LOW);
    return true;
}

bool setPwmOutput(int pin, int duty) {
    int channel = attachPwm(pin);
    if (channel < 0) {
        return false;
    }

    ledcWrite(channel, duty);
    return true;
}

bool readPin(JsonVariantConst value, int& pin) {
    if (!value.is<int>()) {
        return false;
    }

    pin = value.as<int>();
    return true;
}

bool readDuration(JsonVariantConst value, int& durationSeconds) {
    durationSeconds = 0;
    if (value.isNull()) {
        return true;
    }

    if (!value.is<int>()) {
        return false;
    }

    durationSeconds = value.as<int>();
    return durationSeconds >= 0 && durationSeconds <= MAX_DURATION_SECONDS;
}

void onMessage(char* topic, byte* payload, unsigned int length) {
    JsonDocument command;
    DeserializationError error = deserializeJson(command, payload, length);

    if (error) {
        Serial.println("Niepoprawny JSON");
        return;
    }

    String action = command["action"] | "";
    String commandId = command["command_id"] | "";
    bool success = true;
    const char* errorMessage = nullptr;
    int pin = -1;
    int value = 0;
    bool hasValue = false;
    const char* state = nullptr;
    int durationSeconds = 0;

    if (action == "led_on" || action == "led_off") {
        pin = LED_PIN;
        setDigitalOutput(pin, action == "led_on");
        clearTimedOutput(pin);
        state = action == "led_on" ? "high" : "low";
    } else if (action == "get_led_state") {
        pin = LED_PIN;
        hasValue = true;
        value = digitalRead(pin) == HIGH ? 1 : 0;
    } else if (action == "digital_write") {
        if (!readPin(command["pin"], pin) || !isAllowedOutputPin(pin)) {
            success = false;
            errorMessage = "Niedozwolony pin wyjsciowy";
        } else if (!readDuration(command["duration_seconds"], durationSeconds)) {
            success = false;
            errorMessage = "Czas musi byc od 0 do 3600 sekund";
        } else if (!canScheduleTimedOutput(pin)) {
            success = false;
            errorMessage = "Brak wolnego timera dla wyjscia";
        } else {
            String requestedState = command["state"] | "";
            if (requestedState != "high" && requestedState != "low") {
                success = false;
                errorMessage = "Stan musi byc high albo low";
            } else {
                bool high = requestedState == "high";
                setDigitalOutput(pin, high);
                scheduleTimedOutput(pin, TIMED_DIGITAL, durationSeconds);
                state = high ? "high" : "low";
            }
        }
    } else if (action == "digital_read") {
        if (!readPin(command["pin"], pin) || !isAllowedInputPin(pin)) {
            success = false;
            errorMessage = "Niedozwolony pin wejsciowy";
        } else {
            pinMode(pin, INPUT);
            hasValue = true;
            value = digitalRead(pin) == HIGH ? 1 : 0;
        }
    } else if (action == "analog_read") {
        if (!readPin(command["pin"], pin) || !isAllowedAnalogInputPin(pin)) {
            success = false;
            errorMessage = "Niedozwolony pin analogowy";
        } else {
            pinMode(pin, INPUT);
            hasValue = true;
            value = analogRead(pin);
        }
    } else if (action == "pwm_write") {
        if (!readPin(command["pin"], pin) || !isAllowedOutputPin(pin)) {
            success = false;
            errorMessage = "Niedozwolony pin PWM";
        } else if (!command["duty"].is<int>() || command["duty"].as<int>() < 0 || command["duty"].as<int>() > PWM_MAX_DUTY) {
            success = false;
            errorMessage = "PWM musi byc od 0 do 255";
        } else if (!readDuration(command["duration_seconds"], durationSeconds)) {
            success = false;
            errorMessage = "Czas musi byc od 0 do 3600 sekund";
        } else if (!canScheduleTimedOutput(pin)) {
            success = false;
            errorMessage = "Brak wolnego timera dla wyjscia";
        } else {
            int duty = command["duty"].as<int>();
            success = setPwmOutput(pin, duty);
            if (!success) {
                errorMessage = "Brak wolnego kanalu PWM";
            } else {
                scheduleTimedOutput(pin, TIMED_PWM, durationSeconds);
                hasValue = true;
                value = duty;
            }
        }
    } else {
        success = false;
        errorMessage = "Nieznana akcja";
    }

    JsonDocument response;
    response["command_id"] = commandId;
    response["action"] = action;
    response["success"] = success;

    if (pin >= 0) {
        response["pin"] = pin;
    }
    if (state != nullptr) {
        response["state"] = state;
    }
    if (hasValue) {
        response["value"] = value;
    }
    if (durationSeconds > 0 && success) {
        response["duration_seconds"] = durationSeconds;
    }
    if (!success) {
        response["error"] = errorMessage;
    }
    if (action == "led_on" || action == "led_off" || action == "get_led_state") {
        response["led"] = digitalRead(LED_PIN) == HIGH ? "on" : "off";
    }

    String output;
    serializeJson(response, output);
    mqtt.publish(RESPONSE_TOPIC, output.c_str());
    Serial.println(output);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    analogReadResolution(12);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Laczenie Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi polaczone");

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
    serviceTimedOutputs();

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    uint32_t now = millis();
    if (!mqtt.connected() && now - lastMqttConnectAttempt >= MQTT_RECONNECT_INTERVAL_MS) {
        lastMqttConnectAttempt = now;
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
        }
    }

    mqtt.loop();
}