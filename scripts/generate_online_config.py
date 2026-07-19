#!/usr/bin/env python3
"""Generate the C header used by an online Ultra36 menu ROM build."""

import argparse
import json
from pathlib import Path


MAX_NAME_LENGTH = 16


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--bank-count", required=True, choices=("8", "16"))
    parser.add_argument("--names-json", required=True)
    parser.add_argument("--output", required=True, type=Path)
    return parser.parse_args()


def validate_names(raw_names, bank_count):
    expected_count = bank_count - 2

    if not isinstance(raw_names, list):
        raise ValueError("names_json must be a JSON array")
    if len(raw_names) != expected_count:
        raise ValueError(
            f"{bank_count} banks require exactly {expected_count} user ROM names"
        )

    names = []
    for index, raw_name in enumerate(raw_names, start=2):
        if not isinstance(raw_name, str):
            raise ValueError(f"Bank {index} name must be a string")

        name = raw_name.strip()
        if not name:
            raise ValueError(f"Bank {index} name cannot be empty")
        if len(name) > MAX_NAME_LENGTH:
            raise ValueError(
                f"Bank {index} name exceeds {MAX_NAME_LENGTH} characters"
            )
        if any(ord(character) < 32 or ord(character) > 126 for character in name):
            raise ValueError(f"Bank {index} name must contain printable ASCII only")

        names.append(name)

    return names


def render_header(names):
    quoted_names = [json.dumps(name, ensure_ascii=True) for name in names]
    continuation = ", ".join(quoted_names)

    return (
        "/* Generated file. Do not edit or commit. */\n"
        "#ifndef ONLINE_ROM_CONFIG_H\n"
        "#define ONLINE_ROM_CONFIG_H\n\n"
        f"#define NUM_USER_ROMS {len(names)}\n"
        f"#define USER_ROM_NAMES_INIT {continuation}\n\n"
        "#endif\n"
    )


def main():
    args = parse_args()

    try:
        raw_names = json.loads(args.names_json)
        names = validate_names(raw_names, int(args.bank_count))
    except (json.JSONDecodeError, ValueError) as error:
        raise SystemExit(f"Invalid ROM configuration: {error}") from error

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render_header(names), encoding="ascii")
    print(f"Generated configuration for {len(names)} user ROMs")


if __name__ == "__main__":
    main()
