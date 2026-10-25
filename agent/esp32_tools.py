
from mqtt_client import send_command


def get_led_state():
    return send_command("get_led_state")


def turn_led_on():
    return send_command("led_on")


def turn_led_off():
    return send_command("led_off")
