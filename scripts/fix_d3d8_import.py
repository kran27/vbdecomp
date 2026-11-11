"""Map the generated stdcall thunk to the DLL's undecorated export.

Uses the documented COFF short-import NAME_UNDECORATE field:
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-name-type
Only edits the generated d3d8.lib, never the DLL or SDK.
"""
from pathlib import Path
import struct

path = Path(__file__).resolve().parents[1] / "deps/imports/d3d8.lib"
data = bytearray(path.read_bytes())
if data[:8] != b"!<arch>\n":
    raise ValueError("Expected a COFF archive")
offset = 8
changed = 0
while offset < len(data):
    if data[offset + 58:offset + 60] != b"`\n":
        raise ValueError("Invalid COFF member header")
    size = int(data[offset + 48:offset + 58].decode("ascii").strip())
    start = offset + 60
    body = data[start:start + size]
    if len(body) >= 20 and body[:4] == b"\x00\x00\xff\xff":
        machine = struct.unpack_from("<H", body, 6)[0]
        strings = body[20:].split(b"\x00")
        if strings[:2] == [b"_Direct3DCreate8@4", b"d3d8.dll"]:
            if machine != 0x14C:
                raise ValueError("Expected x86 short import")
            flags = struct.unpack_from("<H", body, 18)[0]
            struct.pack_into("<H", data, start + 18, (flags & ~0x1C) | (3 << 2))
            changed += 1
    offset = start + size + (size % 2)
if changed != 1:
    raise ValueError(f"Expected exactly one Direct3DCreate8 import, found {changed}")
path.write_bytes(data)
print("D3D8: _Direct3DCreate8@4 links to Direct3DCreate8 in d3d8.dll")
