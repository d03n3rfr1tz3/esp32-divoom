import csv
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
NAMESPACE = "divoom"
DEFAULT_SIZE = 0x5000
ENCODINGS = {"str": "string", "u16": "u16"}


def write_csv(path, case):
    """writes the case in the csv format the reference tool expects"""
    with open(path, "w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["key", "type", "encoding", "value"])
        writer.writerow([NAMESPACE, "namespace", "", ""])
        for entry in case["entries"]:
            writer.writerow([entry["key"], "data", ENCODINGS[entry["type"]], entry["value"]])


def generate(outdir):
    """turns every case into a reference image built by esp-idf-nvs-partition-gen"""
    outdir.mkdir(parents=True, exist_ok=True)
    cases = json.loads((ROOT / "cases.json").read_text(encoding="utf-8"))

    for case in cases:
        name = case["name"]
        source = outdir / f"{name}.csv"
        write_csv(source, case)

        subprocess.run(
            [sys.executable, "-m", "esp_idf_nvs_partition_gen", "generate",
             "--outdir", str(outdir), str(source), f"{name}.bin",
             hex(case.get("size", DEFAULT_SIZE))],
            check=True, stdout=subprocess.DEVNULL)

        print(f"{name}.bin")


if __name__ == "__main__":
    generate(Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "fixtures")
