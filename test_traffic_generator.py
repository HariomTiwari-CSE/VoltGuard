"""Helpers for building offline Modbus RTU test frames."""


def crc16_modbus(data):
    """Return the Modbus CRC-16 value for a bytes-like object."""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def build_rtu_frame(body):
    """Append a Modbus RTU CRC, transmitted least-significant byte first."""
    crc = crc16_modbus(body)
    return bytes(body) + bytes((crc & 0xFF, crc >> 8))


def make_record(name, frame, expected_valid, expected_error=None):
    record = {
        "case": name,
        "frame_hex": frame.hex(" "),
        "expected_valid": expected_valid,
    }
    if expected_error is not None:
        record["expected_error"] = expected_error
    return record
