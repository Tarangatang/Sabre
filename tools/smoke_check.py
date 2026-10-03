#!/usr/bin/env python3
"""Dependency-free checks that catch drift between the model and controllers."""

from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_JOINTS = [
    *(f"sabre_finger_{finger}_{joint}_joint"
      for finger in (1, 2, 3)
      for joint in ("rotatory", "flexor_1", "flexor_2", "flexor_3")),
    "sabre_thumb_rotatory_joint",
    "sabre_thumb_flexor_1_joint",
    "sabre_thumb_flexor_2_joint",
    "sabre_thumb_flexor_3_joint",
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

    model_sources = "\n".join(
        path.read_text()
        for path in (ROOT / "src/sabre_hand_description/urdf").rglob("*.xacro")
    )
    mesh_paths = set(re.findall(
        r"package://sabre_hand_description/([^\"]+\.STL)", model_sources
    ))
    if len(mesh_paths) != 11:
        errors.append(f"expected 11 unique Allegro mesh references, found {len(mesh_paths)}")
    for relative_mesh in mesh_paths:
        if not (ROOT / "src/sabre_hand_description" / relative_mesh).is_file():
            errors.append(f"missing mesh: {relative_mesh}")

    for attribution in (
        ROOT / "LICENSE",
        ROOT / "third_party/allegro_hand/LICENSE",
        ROOT / "third_party/allegro_hand/NOTICE",
    ):
        if not attribution.is_file():
            errors.append(f"missing licence or attribution: {attribution.relative_to(ROOT)}")

    declared = re.findall(r'<name>([^<]+)</name>', "\n".join(p.read_text() for p in package_files))
    if len(set(declared)) != 3:
        errors.append("ROS package names are not unique")

    if errors:
        print("Smoke check failed:")
        for error in errors:
            print(f"  - {error}")
        return 1

    print("Smoke check passed: 3 packages, valid XML, and 16 aligned joints.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
