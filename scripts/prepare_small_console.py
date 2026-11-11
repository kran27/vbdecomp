"""Prepare the evidenced F3 console formatter from the supplied Small source.

Only printstring/dochar are reused. The caller and buffer callbacks remain game
code. The supplied checkout is read-only; this emits a separately compiled C TU.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "config/small-console.lock.json"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError("Small console adaptation context differs: " + old[:60])
    return text.replace(old, new)


def render(source):
    text = source.decode("utf-8").replace("\r\n", "\n")
    copyright_text = text[:text.index("#if defined __WIN32__")]
    start = text.index("static int dochar(AMX *amx,char ch,cell param)\n{")
    end = text.index("static cell AMX_NATIVE_CALL _print(", start)
    extracted = text[start:end]
    functions = extracted
    functions = replace_once(functions,
        "#if defined FLOATPOINT\n  case 'f': /* 32-bit floating point number */\n"
        "#if !defined FIXEDPOINT\n  case 'r':\n#endif\n"
        "    amx_GetAddr(amx,param,&cptr);\n"
        "    amx_printf(\"%f\",*(float*)cptr);\n    return 1;\n#endif\n",
        "  case 'f': /* Original F3 exposes f, without the upstream r alias. */\n"
        "    amx_GetAddr(amx,param,&cptr);\n"
        "    amx_printf(\"%f\",*(float*)cptr);\n    return 1;\n")
    fixed_start = functions.index("#if defined FIXEDPOINT\n")
    fixed_end = functions.index("  case 's':\n", fixed_start)
    functions = functions[:fixed_start] + functions[fixed_end:]
    functions = replace_once(functions,
        "  case 'x':\n    amx_GetAddr(amx,param,&cptr);\n"
        "    amx_printf(\"%lx\",(long)*cptr);\n    return 1;\n", "")
    # The original callbacks take the active VM for script-error diagnostics.
    functions = functions.replace("amx_putchar(", "F3_DebugPutChar(amx,")
    functions = functions.replace("amx_printf(", "F3_DebugPrintf(amx,")
    header = """#ifndef F3_SMALL_CONSOLE_H
#define F3_SMALL_CONSOLE_H
#include "amx.h"
#ifdef __cplusplus
extern "C" {
#endif
int F3_SmallPrintString(AMX *amx,cell *cstr,cell *params,int num);
void F3_DebugPutChar(AMX *amx,int ch);
void F3_DebugPrintf(AMX *amx,const char *format,...);
#ifdef __cplusplus
}
#endif
#endif
"""
    generated = copyright_text + """/* Generated from the hash-pinned supplied amxcons.c.
 * F3 adaptations: game buffer callbacks; only %,c,d,f,s format cases.
 * See config/small-console.lock.json and retained original evidence.
 */
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include "small_console.h"

static int printstring(AMX *amx,cell *cstr,cell *params,int num);

""" + functions + """
int F3_SmallPrintString(AMX *amx,cell *cstr,cell *params,int num)
{
  return printstring(amx,cstr,params,num);
}
"""
    return {"small_console.c": generated.encode(),
            "small_console.h": header.encode()}, sha(extracted.encode())


def prepare(source_root, destination):
    source_root = Path(source_root).resolve()
    destination = Path(destination).resolve()
    if not destination.is_relative_to(ROOT):
        raise ValueError("Small console outputs must remain in the workspace")
    if destination == source_root or source_root in destination.parents:
        raise ValueError("Small console outputs cannot modify the supplied checkout")
    lock = json.loads(LOCK.read_text())
    source = source_root / "src/amxcons.c"
    raw = source.read_bytes()
    if sha(raw) != lock["source"]["sha256"]:
        raise ValueError("Supplied amxcons.c identity differs")
    outputs, extracted_sha = render(raw)
    if extracted_sha != lock["extracted_functions_sha256"]:
        raise ValueError("Supplied Small console function text differs")
    actual = {name: sha(data) for name, data in outputs.items()}
    if actual != lock["outputs"]:
        raise ValueError("Adapted Small console source identity differs")
    destination.mkdir(parents=True, exist_ok=True)
    for name, data in outputs.items():
        path = destination / name
        if not path.exists() or path.read_bytes() != data:
            path.write_bytes(data)
    receipt = dict(schema_version=1, source_path=str(source), source_sha256=sha(raw),
                   extracted_functions_sha256=extracted_sha,
                   generator_sha256=sha(Path(__file__).read_bytes()),
                   lock_sha256=sha(LOCK.read_bytes()), outputs=actual,
                   compiled_as="C", user_checkout_modified=False,
                   target_code_executed=False,
                   dependency_original_addresses=lock["dependency_original_addresses"],
                   proprietary_progress_credit=False)
    (destination / "source-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    prepare(args.source_root, args.output)


if __name__ == "__main__":
    main()
