
from mqtt_client import send_command

OUTPUT_PINS = [2, 4, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33]
INPUT_PINS = [32, 33, 34, 35, 36, 39]
ANALOG_INPUT_PINS = [32, 33, 34, 35, 36, 39]


def get_led_state():
    return send_command("get_led_state")


def turn_led_on():
    return send_command("led_on")


def turn_led_off():
    return send_command("led_off")


def set_digital_output(pin, state, duration_seconds=None):
    return send_command(
        "digital_write",
        pin=pin,
        state=state,
        duration_seconds=duration_seconds,
    )


def read_digital_input(pin):
    return send_command("digital_read", pin=pin)


def read_analog_input(pin):
    return send_command("analog_read", pin=pin)


def set_pwm_output(pin, duty, duration_seconds=None):
    return send_command(
        "pwm_write",
        pin=pin,
        duty=duty,
        duration_seconds=duration_seconds,
    )
