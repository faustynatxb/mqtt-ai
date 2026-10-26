import os
import sys
import unittest
from pathlib import Path


os.environ.setdefault("MQTT_HOST", "test-host")
os.environ.setdefault("MQTT_USERNAME", "test-user")
os.environ.setdefault("MQTT_PASSWORD", "test-password")
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "agent"))

from mqtt_client import build_command_payload


class BuildCommandPayloadTests(unittest.TestCase):
    def test_includes_action_command_id_and_parameters(self):
        payload = build_command_payload(
            "command-123",
            "digital_write",
            pin=25,
            state="high",
            duration_seconds=10,
        )

        self.assertEqual(
            payload,
            {
                "command_id": "command-123",
                "action": "digital_write",
                "pin": 25,
                "state": "high",
                "duration_seconds": 10,
            },
        )

    def test_omits_optional_parameters_set_to_none(self):
        payload = build_command_payload(
            "command-123",
            "pwm_write",
            pin=26,
            duty=128,
            duration_seconds=None,
        )

        self.assertNotIn("duration_seconds", payload)
        self.assertEqual(payload["duty"], 128)


if __name__ == "__main__":
    unittest.main()
