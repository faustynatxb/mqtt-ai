---
name: esp32-agent-render-deploy
description: Use when deploying ESP32 agent web chat to free Render.
---

# Deploy ESP32 Agent Web Chat to Render

Use this only for the Gradio web-chat deployment in this repository. Work one checkpoint at a time: verify locally, guide the student through Render and GitHub login, configure a free Web Service, then verify the live URL. Never claim a deployment succeeded without reading the Render deploy state and fetching the live app.

## Scope

- The web entrypoint is `app.py`; Gradio is both the chat frontend and Python server.
- The existing agent logic remains in `agent/`.
- Each student deploys their own Render Web Service from their own GitHub fork.
- Use Render's Free Web Service plan. It has 512 MB RAM, 0.1 CPU, 750 instance-hours per workspace each month, and sleeps after 15 idle minutes.
- Treat `agent/.env` as secret. Never read, print, commit, or upload it.
- A free Render Web Service is publicly reachable. Leave Gradio authentication unset for an open class demo; configure it only if the student chooses to restrict access.

## Local checkpoint

1. Read `README.md`, `app.py`, `requirements.txt`, `agent/requirements.txt`, `agent/.env.example`, `agent/agent.py`, and `agent/mqtt_client.py` before editing or deploying.
2. Inspect `git status --short`. Keep unrelated changes out of the deployment; name any unrelated paths before proceeding.
3. Resolve a working Python interpreter. Do not assume the repository `.venv` is valid. Install dependencies into a temporary verification virtual environment if required.
4. Run `python -m compileall -q app.py agent` and import `app` with harmless placeholder environment variables. Confirm that `app.demo` is a Gradio `ChatInterface`.
5. Start `app.py` with placeholder environment variables and `PORT=10000`; verify the Gradio HTTP URL returns 200, inspect the listener command, then stop the local process.

## Student account checkpoint

1. Ask whether the student already has GitHub and Render accounts. If not, direct them to `https://github.com/signup` and `https://dashboard.render.com/register`.
2. The student must complete account creation, email verification, CAPTCHA, and login in their own browser. Never request a password, GitHub token, Render API key, or verification code in chat.
3. Confirm that the student forked the instructor repository into their own GitHub account. Render must be connected to that fork, not to the instructor's repository.

## Render checkpoint

1. Before creating the Web Service, show the student these exact settings and ask for explicit confirmation:
   - service type: `Web Service`
   - runtime: `Python 3`
   - root directory: repository root
   - build command: `pip install -r requirements.txt`
   - start command: `python app.py`
   - compute plan: `Free`
2. Guide the student to connect their GitHub fork in the Render dashboard and create the service. The student performs browser-only account actions themselves.
3. In Render's Environment settings, the student adds their own secret values:
   - `OPENROUTER_API_KEY`
   - `OPENROUTER_MODEL`
   - `MQTT_HOST`
   - `MQTT_PORT`
   - `MQTT_USERNAME`
   - `MQTT_PASSWORD`
   - optional `MQTT_DEVICE_ID`
   - optional `GRADIO_USERNAME` and `GRADIO_PASSWORD` only if the student wants to restrict access
4. Do not transfer values from any local `.env` file. Trigger a manual deploy or wait for Render's automatic deploy after the environment values are saved.

## Verification

1. Read the Render deployment status and logs. The service must start without an import or port-binding error.
2. Fetch the public `onrender.com` URL and confirm HTTP 200 plus the `ESP32 AI Agent` title. If `GRADIO_USERNAME` and `GRADIO_PASSWORD` are configured, confirm the login screen is active.
3. Do not perform a real MQTT command merely as a deployment check. If the student wants to test control, they must explicitly authorize a command against their isolated test device.
4. Report the live URL, deploy status, free-tier sleep limitation, and any remaining student action in four short bullets.

## Pitfalls

- Render Free Web Services sleep after 15 minutes with no inbound traffic. The next visit can take about a minute to wake the app.
- The free filesystem is ephemeral. Do not store uploaded files or a SQLite database there.
- Gradio authentication is optional. Leave `GRADIO_USERNAME` and `GRADIO_PASSWORD` unset when anyone should access the demo; set both only to restrict it.
- Keep Gradio as the first web UI. Add FastAPI only when another client, such as the Chrome extension, actually needs an API.
