
from esp32_tools import (
    get_led_state,
    turn_led_on,
    turn_led_off
)

TOOL_FUNCTIONS = {
    "get_led_state": get_led_state,
    "turn_led_on": turn_led_on,
    "turn_led_off": turn_led_off,
}

DESCRIPTIONS = {
    "get_led_state": "Sprawdza aktualny stan LED na ESP32.",
    "turn_led_on": "Włącza diodę LED na ESP32.",
    "turn_led_off": "Wyłącza diodę LED na ESP32.",
}

TOOLS = [
    {
        "type": "function",
        "function": {
            "name": name,
            "description": description,
            "parameters": {
                "type": "object",
                "properties": {},
                "additionalProperties": False
            }
        }
    }
    for name, description in DESCRIPTIONS.items()
]


def execute_tool(name, arguments):
    if name not in TOOL_FUNCTIONS:
        return {"error": f"Nieznane narzędzie: {name}"}

    if arguments:
        return {"error": "To narzędzie nie przyjmuje argumentów"}

    return TOOL_FUNCTIONS[name]()
