"""Strict Windows x86 PE/COFF adapter for the local progress tracker.

PE functions require an external, hash-pinned chunk map. COFF bounds come from
CodeView procedure lengths or function-definition auxiliary records, never the
next symbol or a whole COMDAT's padded size. See docs/matching-policy.md for limits.
"""
from pathlib import Path
import hashlib
import struct

import pefile


def integer(value):
    return int(value, 0) if isinstance(value, str) else int(value)


class Binary:
    def __init__(self, path, function_map=None):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        self.sections, self.symbols = [], []
        self.image_base = 0
        self.kind = "pe" if self.data.startswith(b"MZ") else "coff"
        if self.kind == "pe":
            self._pe(function_map)
        else:
            if function_map is not None:
                raise ValueError("COFF bounds must come from compiler procedure records")
            self._coff()
        self.abi = {"machine": self.machine, "bits": 32, "endianness": "little",
                    "instruction_mode": "x86-32"}

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > len(self.data):
            raise ValueError("truncated binary extent")
        return self.data[offset:offset + size]

    def _pe(self, function_map):
        try:
            pe = pefile.PE(data=self.data, fast_load=True)
        except pefile.PEFormatError as error:
            raise ValueError(f"invalid PE: {error}") from error
        self.machine = pe.FILE_HEADER.Machine
        if self.machine != 0x14C or pe.OPTIONAL_HEADER.Magic != 0x10B:
            raise ValueError("expected I386 PE32")
        self.image_base = pe.OPTIONAL_HEADER.ImageBase
        for section in pe.sections:
            self.sections.append(dict(name=section.Name.rstrip(b"\0").decode("ascii"),
                address=self.image_base + section.VirtualAddress, offset=section.PointerToRawData,
                size=min(section.SizeOfRawData, section.Misc_VirtualSize),
                flags=section.Characteristics, relocations=[]))
        if function_map is not None:
            if function_map["reference_sha256"] != hashlib.sha256(self.data).hexdigest():
                raise ValueError("external function map SHA256 differs from PE")
            for function in function_map["functions"]:
                if function.get("complete") is not True:
                    raise ValueError("external map must identify complete function chunks")
                entry = dict(function)
                entry["address"] = integer(entry.get("address", entry["id"]))
                chunks = []
                for chunk in entry["chunks"]:
                    address, size = integer(chunk["address"]), integer(chunk["size"])
                    index = self.section_at(address, size)
                    chunks.append(dict(address=address, size=size, section=index,
                                       offset=address - self.sections[index]["address"]))
                    actual = self.read(self.sections[index]["offset"] + chunks[-1]["offset"], size)
                    if hashlib.sha256(actual).hexdigest() != chunk["sha256"]:
                        raise ValueError(f"function chunk SHA256 mismatch: {entry['id']}")
                if not chunks or not any(c["address"] == entry["address"] for c in chunks):
                    raise ValueError("function entry is outside its chunks")
                entry["chunks"] = sorted(chunks, key=lambda c: c["address"])
                entry["size"] = sum(c["size"] for c in chunks)
                self.symbols.append(entry)

    def section_at(self, address, size):
        matches = [i for i, section in enumerate(self.sections)
                   if section["flags"] & 0x20000000 and size > 0
                   and section["address"] <= address
                   and address + size <= section["address"] + section["size"]]
        if len(matches) != 1:
            raise ValueError("function extends outside one file-backed executable section")
        self.read(self.sections[matches[0]]["offset"] + address - self.sections[matches[0]]["address"], size)
        return matches[0]

    def _coff(self):
        if self.data.startswith((b"!<arch>\n", b"\0\0\xff\xff")):
            raise ValueError("archives, import objects, bigobj and LTCG are unsupported")
        machine, count, _, symbol_offset, symbol_count, optional_size, _ = struct.unpack("<HHIIIHH", self.read(0, 20))
        self.machine = machine
        if machine != 0x14C or optional_size or not count:
            raise ValueError("expected standard I386 COFF object")
        string_offset = symbol_offset + 18 * symbol_count
        string_size = struct.unpack("<I", self.read(string_offset, 4))[0]
        if string_size < 4:
            raise ValueError("invalid COFF string table")
        strings = self.read(string_offset, string_size)

        def string(offset):
            if not 4 <= offset < len(strings) or b"\0" not in strings[offset:]:
                raise ValueError("invalid COFF long name")
            return strings[offset:strings.index(0, offset)].decode("utf-8")

        for i in range(count):
            raw = self.read(20 + 40 * i, 40)
            name, _, address, size, offset, reloc_offset, _, reloc_count, _, flags = struct.unpack("<8sIIIIIIHHI", raw)
            name = name.rstrip(b"\0")
            name = string(int(name[1:])) if name.startswith(b"/") else name.decode("ascii")
            if flags & 0x01000000:
                raise ValueError("COFF extended relocation tables are unsupported")
            if offset or not flags & 0x80:  # Uninitialized COFF data has no file payload.
                self.read(offset, size)
            relocs = [struct.unpack("<IIH", self.read(reloc_offset + j * 10, 10)) for j in range(reloc_count)]
            self.sections.append(dict(name=name, address=address, size=size, offset=offset,
                                      flags=flags, relocations=relocs))
        symbols, i = {}, 0
        while i < symbol_count:
            raw = self.read(symbol_offset + i * 18, 18)
            name, value, section, typ, storage, aux = struct.unpack("<8sIhHBB", raw)
            name = string(struct.unpack_from("<I", name, 4)[0]) if name[:4] == b"\0" * 4 else name.rstrip(b"\0").decode("utf-8")
            self.read(symbol_offset + (i + 1) * 18, aux * 18)
            if i + aux >= symbol_count:
                raise ValueError("truncated COFF auxiliary record")
            symbols[i] = dict(name=name, value=value, section=section, type=typ, storage=storage,
                              auxiliary=self.read(symbol_offset + (i + 1) * 18, aux * 18))
            i += aux + 1
        procedures = {}
        defined = [s for s in symbols.values() if s["type"] == 0x20
                   and 0 < s["section"] <= len(self.sections)
                   and self.sections[s["section"] - 1]["flags"] & 0x20000000]
        # VC7 supplies authoritative COFF function-definition auxiliary lengths.
        # They are sufficient without parsing its older optional debug records.
        aux_complete = bool(defined) and all(s["storage"] in (2, 3) and len(s["auxiliary"]) >= 8
            and struct.unpack_from("<I", s["auxiliary"], 4)[0] > 0 for s in defined)
        self.boundary_source = "coff-function-auxiliary" if aux_complete else "codeview-and-auxiliary"
        for section in ([] if aux_complete else self.sections):
            if section["name"] != ".debug$S":
                continue
            data = self.read(section["offset"], section["size"])
            if len(data) < 4 or struct.unpack_from("<I", data)[0] != 4:
                raise ValueError("unsupported CodeView signature (compile with /Z7)")
            relocations = {offset: (index, typ) for offset, index, typ in section["relocations"]}
            offset = 4
            while offset < len(data):
                if offset + 8 > len(data):
                    raise ValueError("truncated CodeView subsection")
                kind, length = struct.unpack_from("<II", data, offset)
                start, end = offset + 8, offset + 8 + length
                # VC7 per-function streams leave the final symbol subsection's
                # length zero. Its records occupy the rest of this section;
                # procedure records, not section/code padding, still give bounds.
                if kind == 0xF1 and length == 0:
                    end = len(data)
                if end > len(data):
                    raise ValueError("truncated CodeView subsection data")
                if kind == 0xF1:  # DEBUG_S_SYMBOLS
                    pos = start
                    while pos < end:
                        if pos + 4 > end:
                            raise ValueError("truncated CodeView symbol")
                        size, typ = struct.unpack_from("<HH", data, pos)
                        next_pos = pos + size + 2
                        if size < 2 or next_pos > end:
                            raise ValueError("invalid CodeView symbol size")
                        if typ in (0x110F, 0x1110, 0x1146, 0x1147):
                            payload = pos + 4
                            if payload + 35 > next_pos:
                                raise ValueError("truncated CodeView procedure")
                            code_size = struct.unpack_from("<I", data, payload + 12)[0]
                            code_offset = struct.unpack_from("<I", data, payload + 28)[0]
                            segment = struct.unpack_from("<H", data, payload + 32)[0]
                            for delta, expected in ((28, 0xB), (32, 0xA)):
                                relocation = relocations.get(payload + delta)
                                if relocation:
                                    index, reloc_type = relocation
                                    if reloc_type != expected or index not in symbols:
                                        raise ValueError("unsupported CodeView procedure relocation")
                                    symbol = symbols[index]
                                    if delta == 28:
                                        code_offset += symbol["value"]
                                    else:
                                        segment += symbol["section"]
                            key = segment, code_offset
                            # VC7 can retain procedure records for undefined
                            # template members. Section0 supplies no emitted
                            # code or usable bounds, even when lengths differ.
                            if segment == 0:
                                pos = next_pos
                                continue
                            if key in procedures and procedures[key] != code_size:
                                raise ValueError("conflicting compiler function bounds")
                            procedures[key] = code_size
                        pos = next_pos
                offset = (end + 3) & ~3
        for symbol in symbols.values():
            if symbol["type"] != 0x20 or not 0 < symbol["section"] <= len(self.sections):
                continue
            index = symbol["section"] - 1
            section = self.sections[index]
            if not section["flags"] & 0x20000000:
                continue
            size = procedures.get((symbol["section"], symbol["value"]))
            if symbol["storage"] in (2, 3) and symbol["auxiliary"]:
                aux_size = struct.unpack_from("<I", symbol["auxiliary"], 4)[0]
                if size is not None and aux_size and size != aux_size:
                    raise ValueError("COFF/CodeView function size disagreement")
                size = size or aux_size
            if not size:
                continue  # Never infer a function length from its next symbol.
            if symbol["value"] + size > section["size"]:
                raise ValueError("COFF function extends outside its section")
            self.symbols.append(dict(name=symbol["name"], address=symbol["value"], size=size,
                chunks=[dict(address=symbol["value"], offset=symbol["value"], size=size, section=index)]))

    def functions(self):
        return self.symbols

    def body(self, symbol):
        return b"".join(self.read(self.sections[c["section"]]["offset"] + c["offset"], c["size"])
                        for c in symbol["chunks"])

    def relocations(self, symbol):
        if self.kind == "pe":
            return []  # Linked file bytes at the preferred VA; nothing is rebased/masked.
        affected = []
        widths = {0x0: 0, 0x1: 2, 0x2: 2, 0x6: 4, 0x7: 4, 0x9: 2, 0xA: 2, 0xB: 4, 0xC: 1, 0xD: 4, 0x14: 4}
        for chunk in symbol["chunks"]:
            for offset, index, typ in self.sections[chunk["section"]]["relocations"]:
                width = widths.get(typ, 8)  # Unknown encodings conservatively block credit.
                if width and offset < chunk["offset"] + chunk["size"] and offset + width > chunk["offset"]:
                    affected.append({"offset": offset, "symbol_index": index, "type": typ})
        return affected
