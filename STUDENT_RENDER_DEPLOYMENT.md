# Student guide: deploy the ESP32 web chat to Render

Each student deploys an independent Render service from their own GitHub fork. Do not share an instructor account, API key, MQTT password, or physical ESP32 with the public internet.

## Before starting

- Install Hermes Agent: https://hermes-agent.nousresearch.com/docs/getting-started/installation
- Clone this repository after the instructor has pushed the latest changes:

  ```sh
  git clone https://github.com/faustyna77/esp32_mqtt-agent-harness.git
  cd esp32_mqtt-agent-harness
  ```

- Start a new Hermes session in this directory and trust the repository skill once:

  ```sh
  hermes skills trust
  hermes
  ```

## Run the guided deployment

In Hermes, enter:

```text
/esp32-agent-render-deploy
```

The agent verifies the project locally, then guides the student through the following checkpoints.

1. If needed, the student creates GitHub and Render accounts at `https://github.com/signup` and `https://dashboard.render.com/register`.
2. The student completes email verification, CAPTCHA, and browser login themselves. Passwords, access tokens, and verification codes never belong in chat.
3. The student forks the instructor repository into their own GitHub account.
4. In Render, the student creates a `Web Service` from that fork with:
   - runtime: `Python 3`
   - build command: `pip install -r requirements.txt`
   - start command: `python app.py`
   - compute plan: `Free`
5. In Render Environment, the student adds their own values for:
   - `OPENROUTER_API_KEY`
   - `OPENROUTER_MODEL`
   - `MQTT_HOST`
   - `MQTT_PORT`
   - `MQTT_USERNAME`
   - `MQTT_PASSWORD`
   - optional `MQTT_DEVICE_ID`
   - optional `GRADIO_USERNAME` and `GRADIO_PASSWORD` only when they want to restrict access
6. The agent verifies the deployed public `onrender.com` URL. It does not send a real MQTT command as part of deployment verification.

## Important limits

- Render Free Web Services have 512 MB RAM and 0.1 CPU. They sleep after 15 minutes without traffic; the next visit can take about a minute to wake the app.
- The filesystem is temporary. Do not store documents or a SQLite database on the service.
- A Render Web Service has a public URL. Leave the Gradio credentials unset when the class should access the demo; set them only to restrict access.
- For a class, give each student a separate test device or isolated MQTT topic and credentials.
