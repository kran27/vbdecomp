"""Keep the supplied Bink DLL's decorated names in the generated import library.

LIB adds the x86 C prefix to DEF names; its default IMPORT_NAME_NOPREFIX would
then remove that prefix from the DLL lookup. These exports retain it. Set the
documented IMPORT_NAME(1) field in generated metadata only, never in a DLL/SDK.
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-name-type
"""
import hashlib,json,struct
from pathlib import Path
import pefile
from workspace_config import configuration

ROOT=Path(__file__).resolve().parents[1]
EXPECTED={'_BinkWait@4','_BinkDoFrame@4','_BinkCopyToBuffer@28',
 '_BinkNextFrame@4','_BinkOpen@8','_BinkGetError@0','_BinkOpenDirectSound@4',
 '_BinkSetSoundSystem@8','_BinkClose@4'}

def fix(data,expected):
 if data[:8]!=b'!<arch>\n':raise ValueError('Expected a COFF archive')
 data=bytearray(data);offset=8;found=[]
 while offset<len(data):
  if data[offset+58:offset+60]!=b'`\n':raise ValueError('Invalid import member header')
  size=int(data[offset+48:offset+58].decode('ascii').strip());start=offset+60;body=data[start:start+size]
  if len(body)!=size:raise ValueError('Truncated import member')
  if body[:4]==b'\0\0\xff\xff':
   if len(body)<20 or struct.unpack_from('<H',body,6)[0]!=0x14C:raise ValueError('Expected x86 short import')
   strings=body[20:].split(b'\0')
   if len(strings)!=3 or strings[1]!=b'binkw32.dll' or strings[2]:raise ValueError('Unexpected short import DLL/strings')
   name=strings[0].decode('ascii');flags=struct.unpack_from('<H',body,18)[0]
   if name not in expected or name in found or flags&3 or flags&~0x1C or flags>>2 not in (1,2):raise ValueError('Unexpected Bink symbol/type/name policy')
   if struct.unpack_from('<I',body,12)[0]!=len(body)-20:raise ValueError('Short import payload size differs')
   struct.pack_into('<H',data,start+18,(flags&~0x1C)|(1<<2));found.append(name)
  offset=start+size+(size%2)
 if offset!=len(data) or set(found)!=expected:raise ValueError('Expected exactly the nine original Bink imports')
 return bytes(data)

def main():
 game=configuration()['gameRoot']
 target=game/'F3.exe';dll=game/'binkw32.dll';library=ROOT/'deps/imports/binkw32.lib'
 if hashlib.sha256(target.read_bytes()).hexdigest()!='4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87':raise ValueError('Original target differs')
 with pefile.PE(str(target)) as pe:
  expected={i.name.decode('ascii') for d in pe.DIRECTORY_ENTRY_IMPORT if d.dll.lower()==b'binkw32.dll' for i in d.imports}
 with pefile.PE(str(dll)) as pe:
  if pe.FILE_HEADER.Machine!=0x14C or expected!=EXPECTED or not expected<={s.name.decode('ascii') for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name}:raise ValueError('Original Bink import/export ABI differs')
 library.write_bytes(fix(library.read_bytes(),expected))
 inputs=[target,dll,ROOT/'deps/imports/binkw32.def',ROOT/'deps/msvc70/bin/LIB.EXE',Path(__file__).resolve()]
 receipt=dict(target_code_executed=False,metadata_only=True,dll_exports_verified=sorted(expected),
  import_name_type='IMPORT_NAME',inputs={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},library_sha256=hashlib.sha256(library.read_bytes()).hexdigest())
 (ROOT/'deps/imports/binkw32-imports.json').write_text(json.dumps(receipt,indent=2)+'\n')
 print('Bink: all nine actual decorated DLL names preserved; no DLL/code changes.')

if __name__=='__main__':main()
