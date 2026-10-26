from esp32_tools import (
    ANALOG_INPUT_PINS,
    INPUT_PINS,
    OUTPUT_PINS,
    get_led_state,
    read_analog_input,
    read_digital_input,
    set_digital_output,
    set_pwm_output,
    turn_led_off,
    turn_led_on,
)


TOOL_FUNCTIONS = {
    "get_led_state": get_led_state,
    "turn_led_on": turn_led_on,
    "turn_led_off": turn_led_off,
    "set_digital_output": set_digital_output,
    "read_digital_input": read_digital_input,
    "read_analog_input": read_analog_input,
    "set_pwm_output": set_pwm_output,
}


TOOLS = [
    {
        "type": "function",
        "function": {
            "name": "get_led_state",
            "description": "Sprawdza stan wbudowanej diody LED.",
            "parameters": {
                "type": "object",
                "properties": {},
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "turn_led_on",
            "description": "Włącza wbudowaną diodę LED.",
            "parameters": {
                "type": "object",
                "properties": {},
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "turn_led_off",
            "description": "Wyłącza wbudowaną diodę LED.",
            "parameters": {
                "type": "object",
                "properties": {},
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "set_digital_output",
            "description": "Ustawia wybrany GPIO na stan wysoki albo niski, opcjonalnie na ograniczony czas.",
            "parameters": {
                "type": "object",
                "properties": {
                    "pin": {
                        "type": "integer",
                        "enum": OUTPUT_PINS,
                        "description": "Dozwolony numer GPIO wyjścia.",
                    },
                    "state": {
                        "type": "string",
                        "enum": ["high", "low"],
                    },
                    "duration_seconds": {
                        "type": "integer",
                        "minimum": 1,
                        "maximum": 3600,
                        "description": "Opcjonalny czas działania; po nim GPIO wraca do stanu niskiego.",
                    },
                },
                "required": ["pin", "state"],
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "read_digital_input",
            "description": "Odczytuje stan cyfrowy przycisku lub innego wejścia GPIO.",
            "parameters": {
                "type": "object",
                "properties": {
                    "pin": {
                        "type": "integer",
                        "enum": INPUT_PINS,
                        "description": "Dozwolony numer GPIO wejścia.",
                    },
                },
                "required": ["pin"],
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "read_analog_input",
            "description": "Odczytuje surową wartość analogową 0-4095 z wejścia ADC1.",
            "parameters": {
                "type": "object",
                "properties": {
                    "pin": {
                        "type": "integer",
                        "enum": ANALOG_INPUT_PINS,
                        "description": "Dozwolony numer GPIO wejścia ADC1.",
                    },
                },
                "required": ["pin"],
                "additionalProperties": False,
            },
        },
    },
    {
        "type": "function",
        "function": {
            "name": "set_pwm_output",
            "description": "Ustawia PWM 0-255 na GPIO, opcjonalnie na ograniczony czas.",
            "parameters": {
                "type": "object",
                "properties": {
                    "pin": {
                        "type": "integer",
                        "enum": OUTPUT_PINS,
                        "description": "Dozwolony numer GPIO wyjścia PWM.",
                    },
                    "duty": {
                        "type": "integer",
                        "minimum": 0,
                        "maximum": 255,
                    },
                    "duration_seconds": {
                        "type": "integer",
                        "minimum": 1,
                        "maximum": 3600,
                        "description": "Opcjonalny czas działania; po nim PWM wraca do 0.",
                    },
                },
                "required": ["pin", "duty"],
                "additionalProperties": False,
            },
        },
    },
]


def execute_tool(name, arguments):
    if name not in TOOL_FUNCTIONS:
        return {"error": f"Nieznane narzędzie: {name}"}

    return TOOL_FUNCTIONS[name](**arguments)