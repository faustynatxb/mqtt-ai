#pragma once

// Dane sieci Wi-Fi
constexpr char WIFI_SSID[] = "TWOJA_SIEC_WIFI";
constexpr char WIFI_PASSWORD[] = "TWOJE_HASLO_WIFI";

// Dane brokera EMQX Cloud
constexpr char MQTT_HOST[] = "TWOJ_HOST.emqxsl.com";
constexpr int MQTT_PORT = 8883;

constexpr char MQTT_USER[] = "esp32-device";
constexpr char MQTT_PASS[] = "TWOJE_HASLO_MQTT";

// Certyfikat CA EMQX
constexpr char ROOT_CA[] = R"PEM(
-----BEGIN CERTIFICATE-----
TUTAJ_WKLEJ_ZAWARTOSC_CERTYFIKATU
-----END CERTIFICATE-----
)PEM";