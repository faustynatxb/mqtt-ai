import os
import sys
import unittest
from pathlib import Path
from unittest.mock import patch


os.environ.setdefault("MQTT_HOST", "test-host")
os.environ.setdefault("MQTT_USERNAME", "test-user")
os.environ.setdefault("MQTT_PASSWORD", "test-password")
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "agent"))

import esp32_tools


class Esp32ToolTests(unittest.TestCase):
    @patch("esp32_tools.send_command")
    def test_sets_digital_output_for_optional_duration(self, send_command):
        esp32_tools.set_digital_output(25, "high", 10)

        send_command.assert_called_once_with(
            "digital_write",
            pin=25,
            state="high",
            duration_seconds=10,
        )

    @patch("esp32_tools.send_command")
    def test_reads_digital_input(self, send_command):
        esp32_tools.read_digital_input(34)

        send_command.assert_called_once_with("digital_read", pin=34)

    @patch("esp32_tools.send_command")
    def test_reads_analog_input(self, send_command):
        esp32_tools.read_analog_input(34)

        send_command.assert_called_once_with("analog_read", pin=34)

    @patch("esp32_tools.send_command")
    def test_sets_pwm_output_for_optional_duration(self, send_command):
        esp32_tools.set_pwm_output(26, 128, 5)

        send_command.assert_called_once_with(
            "pwm_write",
            pin=26,
            duty=128,
            duration_seconds=5,
        )


if __name__ == "__main__":
    unittest.main()
