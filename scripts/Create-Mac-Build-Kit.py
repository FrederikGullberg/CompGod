"""Package portable source and pinned JUCE; preserve Unix modes and shell line endings."""
from pathlib import Path
import argparse
import stat
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument("--juce", type=Path, required=True)
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
juce = args.juce.resolve()
if not (juce / "CMakeLists.txt").is_file() or not (juce / "LICENSE.md").is_file():
    parser.error("--juce must name the JUCE 8.0.6 source folder")
output = root / "dist" / "CompGod-Mac-Build-Kit.zip"
output.parent.mkdir(exist_ok=True)
prefix = "CompGod-Mac-Build-Kit/"
files = []
for name in ("CMakeLists.txt", "CMakePresets.json", "README.md", "INSTALL-MAC.txt", "INSTALL.txt", "Build-Mac.command", ".gitignore"):
    files.append((root / name, name))
for directory in ("Source", "Tests", "scripts", "docs", ".github"):
    for path in sorted((root / directory).rglob("*")):
        if path.is_file() and "__pycache__" not in path.parts:
            files.append((path, path.relative_to(root).as_posix()))
for path in sorted(juce.rglob("*")):
    if path.is_file() and ".git" not in path.relative_to(juce).parts:
        files.append((path, "Dependencies/JUCE/" + path.relative_to(juce).as_posix()))
with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path, relative in files:
        data = path.read_bytes()
        executable = path.suffix in (".sh", ".command") or data.startswith(b"#!")
        if executable:
            data = data.replace(b"\r\n", b"\n")
        info = zipfile.ZipInfo(prefix + relative)
        info.create_system = 3
        info.external_attr = (stat.S_IFREG | (0o755 if executable else 0o644)) << 16
        info.compress_type = zipfile.ZIP_DEFLATED
        archive.writestr(info, data)
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None, "Archive integrity failed"
    for name in ("Build-Mac.command", "scripts/Package-Mac.sh", "scripts/Install-Mac.command"):
        info = archive.getinfo(prefix + name)
        assert (info.external_attr >> 16) & stat.S_IXUSR, "Missing execute bit"
        assert b"\r" not in archive.read(info), "Shell script has Windows line endings"
print(f"Created {output} ({output.stat().st_size:,} bytes, {len(files)} files)")
