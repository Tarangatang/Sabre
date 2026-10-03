#!/usr/bin/env python3
"""Dependency-free checks that catch drift between the model and controllers."""

from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_JOINTS = [
    f"{finger}_joint_{number}"
    for finger in ("thumb", "index", "middle", "ring", "little")
    for number in (1, 2, 3)
]


def main() -> int:
    errors = []
    package_files = list((ROOT / "src").glob("*/package.xml"))
    if len(package_files) != 3:
        errors.append(f"expected 3 ROS packages, found {len(package_files)}")

    xml_files = package_files + [
        ROOT / "src/sabre_hand_description/worlds/empty.sdf",
        ROOT / "src/sabre_hand_description/urdf/sabre_hand.urdf.xacro",
    ]
    for xml_file in xml_files:
        try:
            ET.parse(xml_file)
        except (ET.ParseError, OSError) as error:
            errors.append(f"invalid XML in {xml_file.relative_to(ROOT)}: {error}")

    controller_text = (ROOT / "src/sabre_hand_bringup/config/controllers.yaml").read_text()
    model_text = (ROOT / "src/sabre_hand_description/urdf/sabre_hand.urdf.xacro").read_text()
    cpp_text = (ROOT / "src/sabre_hand_control/src/hand_model.cpp").read_text()
    for joint in EXPECTED_JOINTS:
        if controller_text.count(f"- {joint}") != 1:
            errors.append(f"controller list is missing or duplicates {joint}")
        if f'joint="{joint}"' not in model_text:
            errors.append(f"ros2_control model is missing {joint}")
        if f'"{joint}"' not in cpp_text:
            errors.append(f"C++ joint list is missing {joint}")

    declared = re.findall(r'<name>([^<]+)</name>', "\n".join(p.read_text() for p in package_files))
    if len(set(declared)) != 3:
        errors.append("ROS package names are not unique")

    if errors:
        print("Smoke check failed:")
        for error in errors:
            print(f"  - {error}")
        return 1

    print("Smoke check passed: 3 packages, valid XML, and 15 aligned joints.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
