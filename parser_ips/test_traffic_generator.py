"""Helpers for building offline Modbus RTU test frames."""

import argparse
import json
import random


MAX_VALID_FRAMES = 100


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


def generate_records(count, seed):
    rng = random.Random(seed)
    records = []

    for index in range(count):
        unit_id = rng.randint(1, 247)
        start_address = rng.randint(1, 4095)
        quantity = rng.randint(1, 10)
        read_request = bytes(
            (
                unit_id,
                0x03,
                start_address >> 8,
                start_address & 0xFF,
                quantity >> 8,
                quantity & 0xFF,
            )
        )
        frame = build_rtu_frame(read_request)
        records.append(make_record(f"valid_read_{index + 1}", frame, True))

    # Corrupt only the CRC to exercise checksum validation.
    corrupt_crc = bytearray(build_rtu_frame(bytes((1, 0x03, 0, 1, 0, 2))))
    corrupt_crc[-1] ^= 0xFF
    records.append(
        make_record("invalid_crc", corrupt_crc, False, "invalid RTU CRC")
    )

    # Keep a non-zero address byte so this short RTU frame is not mistaken
    # for a Modbus TCP header by parsers that distinguish formats by prefix.
    malformed_pdu = build_rtu_frame(bytes((1, 0x03, 0, 1, 0)))
    records.append(
        make_record(
            "malformed_read_request",
            malformed_pdu,
            False,
            "invalid read payload",
        )
    )
    return records


def main():
    parser = argparse.ArgumentParser(
        description="Print offline Modbus RTU test frames as JSON Lines."
    )
    parser.add_argument(
        "--count",
        type=int,
        default=10,
        help=f"number of valid read frames (0-{MAX_VALID_FRAMES}, default: 10)",
    )
    parser.add_argument(
        "--seed", type=int, default=7, help="random seed for repeatable output"
    )
    args = parser.parse_args()

    if not 0 <= args.count <= MAX_VALID_FRAMES:
        parser.error(f"--count must be between 0 and {MAX_VALID_FRAMES}")

    for record in generate_records(args.count, args.seed):
        print(json.dumps(record))


if __name__ == "__main__":
    main()
