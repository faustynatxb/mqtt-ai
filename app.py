import os
import sys
from pathlib import Path

import gradio as gr

sys.path.insert(0, str(Path(__file__).parent / "agent"))

try:
    from agent import ESP32Agent
except KeyError as exc:
    ESP32Agent = None
    CONFIGURATION_ERROR = f"Brakuje sekretu: {exc.args[0]}"
else:
    CONFIGURATION_ERROR = None


def text_content(content):
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        return "".join(
            part.get("text", "") if isinstance(part, dict) else str(part)
            for part in content
        )
    return str(content)


def respond(message, history):
    if CONFIGURATION_ERROR:
        return f"{CONFIGURATION_ERROR}. Ustaw go w Settings → Secrets i uruchom Space ponownie."

    agent = ESP32Agent()
    agent.messages.extend(
        {
            "role": item["role"],
            "content": text_content(item["content"]),
        }
        for item in history
        if item["role"] in {"user", "assistant"}
    )
    return agent.chat(message)


demo = gr.ChatInterface(
    respond,
    title="ESP32 AI Agent",
    description="Napisz po polsku, co agent ma zrobić z LED lub wybranym GPIO.",
    examples=[
        "ustaw GPIO 25 na wysoki przez 10 sekund",
        "ustaw PWM 128 na GPIO 26 przez 5 sekund",
        "odczytaj wejście analogowe z GPIO 34",
    ],
)

if __name__ == "__main__":
    demo.launch(
        server_name="0.0.0.0",
        server_port=int(os.getenv("PORT", "7860")),
    )
