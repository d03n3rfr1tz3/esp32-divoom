Import("env")

import json
import shutil
from os.path import basename, isfile, join


def offset_of(value):
    """FLASH_EXTRA_IMAGES carries offsets as hex strings, older frameworks used ints"""
    return value if isinstance(value, int) else int(env.subst(value), 16)


def collect_flash_images(source, target, env):
    """puts every flash image next to firmware.bin and records offset and name in parts.json"""
    build_dir = env.subst("$BUILD_DIR")
    parts = [{"offset": offset_of("$ESP32_APP_OFFSET"), "path": "firmware.bin"}]

    for offset, image in env.get("FLASH_EXTRA_IMAGES", []):
        path = env.subst(image)
        if not isfile(path): raise Exception("flash image missing: " + path)

        name = basename(path)
        destination = join(build_dir, name)
        if path != destination: shutil.copyfile(path, destination)

        parts.append({"offset": offset_of(offset), "path": name})

    parts.sort(key=lambda part: part["offset"])
    with open(join(build_dir, "parts.json"), "w") as handle:
        json.dump(parts, handle, indent=4)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", collect_flash_images)
