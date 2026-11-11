"""Verify the original executable and DLL imports without executing them."""
import argparse
import json
from pathlib import Path
import pefile
from dependency_bundle import ROOT, digest
from setup_inputs import verify_game
from workspace_config import configuration


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generate-defs', action='store_true')
    args = parser.parse_args()
    game = configuration()['gameRoot']
    spec = json.loads((ROOT / 'config/game-download.json').read_bytes())
    verify_game(game, spec)
    with pefile.PE(str(game / 'F3.exe')) as pe:
        if pe.FILE_HEADER.Machine != 0x14c or pe.OPTIONAL_HEADER.ImageBase != 0x400000:
            raise ValueError('Original target machine/image base differs')
        imports = {row.dll.decode('ascii').lower(): [item.name.decode('ascii') if item.name else item.ordinal for item in row.imports] for row in pe.DIRECTORY_ENTRY_IMPORT}
    output = ROOT / 'deps/imports'
    output.mkdir(parents=True, exist_ok=True)
    for name in ('granny2.dll', 'binkw32.dll'):
        with pefile.PE(str(game / name)) as pe:
            if pe.FILE_HEADER.Machine != 0x14c:
                raise ValueError('Expected original x86 DLL: ' + name)
            exports = {item.name.decode('ascii') for item in pe.DIRECTORY_ENTRY_EXPORT.symbols if item.name}
        required = imports[name]
        if not set(required) <= exports:
            raise ValueError('Original DLL export set differs: ' + name)
        if args.generate_defs:
            if any(not isinstance(symbol, str) or not symbol.startswith('_') or '@' not in symbol for symbol in required):
                raise ValueError('Unexpected original stdcall names: ' + name)
            lines = [symbol[1:] for symbol in required]
            (output / (Path(name).stem + '.def')).write_text('LIBRARY ' + name + '\nEXPORTS\n' + ''.join('    ' + symbol + '\n' for symbol in lines), encoding='ascii')
    if imports['d3d8.dll'] != ['Direct3DCreate8']:
        raise ValueError('Unexpected original D3D8 entry points')
    if args.generate_defs:
        (output / 'd3d8.def').write_text('LIBRARY d3d8.dll\nEXPORTS\n    Direct3DCreate8@4\n', encoding='ascii')
    report = {'target_sha256': digest(game / 'F3.exe'), 'imports': imports, 'executed': False}
    destination = ROOT / 'build/environment-report.json'
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('Verified original game imports; generated metadata uses the VC7 librarian')


if __name__ == '__main__':
    main()
