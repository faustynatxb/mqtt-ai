# ESP32 MQTT AI Agent Harness

ESP32 LED controller driven by a Python AI agent through MQTT. The agent uses an OpenRouter-compatible model to choose a command; the MQTT client delivers it to the ESP32 and waits for a correlated response.

## Components

- `src/main.cpp` — ESP32 firmware: Wi-Fi, TLS, MQTT, and LED control.
- `include/secrets.example.h` — template for local Wi-Fi, MQTT, and CA-certificate configuration.
- `agent/main.py` — interactive AI agent.
- `agent/mqtt_client.py` — MQTT command/response client.
- `agent/esp32_tools.py` — agent tools mapping to MQTT commands.

## ESP32 setup

1. Copy `include/secrets.example.h` to `include/secrets.h`.
2. Fill in the Wi-Fi, MQTT broker, and CA certificate values locally.
3. Build and upload:

   ```sh
   pio run --target upload
   ```

4. Watch startup and MQTT connection logs:

   ```sh
   pio device monitor --baud 115200
   ```

The firmware subscribes to `devices/esp32-001/commands` and responds on `devices/esp32-001/responses`. Supported actions are `led_on`, `led_off`, and `get_led_state`.

## Python agent setup

```sh
cd agent
python -m venv ../.venv
source ../.venv/bin/activate
pip install -r requirements.txt
cp .env.example .env
```

Set the OpenRouter and MQTT values in `agent/.env`, then run:

```sh
python main.py
```

Examples: `włącz LED`, `wyłącz LED`, `jaki jest stan LED`.

## GPIO controlled by the agent

The agent can translate Polish natural-language requests into MQTT tool calls:

- digital output: `ustaw GPIO 25 na wysoki przez 10 sekund`;
- PWM output from `0` to `255`: `ustaw PWM 128 na GPIO 26 przez 5 sekund`;
- digital input: `odczytaj przycisk z GPIO 34`;
- analog ADC1 input from `0` to `4095`: `odczytaj wejście analogowe z GPIO 34`.

Allowed output/PWM pins are `2, 4, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33`. Allowed digital and analog input pins are `32, 33, 34, 35, 36, 39`. Timed digital outputs return to LOW and timed PWM outputs return to `0`; duration is limited to 3600 seconds. The firmware validates pins and durations independently of the LLM.

The current PWM tool is for LEDs or motor-driver inputs. A servo needs a separate 50 Hz servo-specific command and power wiring; do not attach it directly to ESP32 3.3 V power.

## Web chat and Render

Run the Gradio chat locally from the repository root:

```sh
pip install -r requirements.txt
python app.py
```

For the guided free Render deployment, start Hermes in this repository and run `/esp32-agent-render-deploy`. Student instructions are in `STUDENT_RENDER_DEPLOYMENT.md`.

## Security

Never commit `include/secrets.h`, `.env` files, certificates, or private keys. They are excluded by `.gitignore`; only the example configuration files belong in the repository.
