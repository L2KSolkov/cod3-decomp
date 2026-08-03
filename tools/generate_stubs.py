#!/usr/bin/env python3
"""
Generate stub .cpp files for each library from MANIFEST_CLASSIFIED.tsv.
Each stub file contains every non-inline function as a log+break stub.
Inline functions (f i / COMDAT) are NOT stubbed — they go into headers.
"""

import os
import csv
import re
from collections import defaultdict

MANIFEST_IN = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'MANIFEST_CLASSIFIED.tsv')
STUBS_DIR = os.path.join(os.path.dirname(__file__), '..', 'src')

# Map lib categories to source directories for game/engine libs
# Game objects (no lib) map to game subdirs
GAME_OBJ_DIRS = {
    'shell.o': 'game/main',
    'scr.o': 'game/script',
    'mp.o': 'game/mp',
    'g.o': 'game/logic',
    'mp_shell.o': 'game/mpshell',
    'game.o': 'game/game',
    'core.o': 'game/core',
    'game2.o': 'game/game2',
    'render.o': 'game/render',
    'anim.o': 'game/anim',
    'mp_actors.o': 'game/mpactors',
    'cg.o': 'game/cg',
    'cl.o': 'game/cl',
    'streamer.o': 'game/streamer',
    'physics.o': 'game/physics',
    'game_xbox.o': 'game/platform_xbox',
    'sv.o': 'game/sv',
    'mp_xbox.o': 'game/platform_xbox',
    'nextgen.o': 'game/nextgen',
    'CallFunctor.o': 'game/core',
}

# Engine lib → source directory
ENGINE_LIB_DIRS = {
    'MPBrocCore_xboxd': 'broc',
    'mp_level.xboxd': 'broc',
    'phys_xboxr': 'physics',
    'render_xboxr': 'render',
    'ngl_xboxr': 'ngl',
    'nal_xboxr': 'animation',
    'nsl_xboxr': 'sound',
    'nfl_xboxr': 'filesystem',
    'nvl_xboxr': 'nvl',
    'aeps_xboxr': 'aeps',
    'mem_mp_xboxr': 'core/memory',
    'core_xboxr': 'core',
    'tl_xboxr': 'core',
    'cdl_xboxr': 'core',
    'controller_xboxr': 'input',
    'jobqueue_xboxr': 'threading',
    'peripherals_xboxr': 'platform_xbox',
    'apk_xboxr': 'filesystem',
    'inplace_xboxr': 'filesystem',
    'bdCore': 'bd',
    'bdConnection': 'bd',
    'bdNet': 'bd',
    'bdPeer': 'bd',
    'bdSocket': 'bd',
    'bdPlatform': 'bd',
    'zlib_xboxr': 'thirdparty/zlib',
}


def demangle_simple(name):
    """Extract a readable signature from a mangled name."""
    # Remove leading ? for basic readability
    if name.startswith('?'):
        s = name[1:]
    else:
        s = name
    # Shorten very long template names
    if len(s) > 100:
        s = s[:97] + '...'
    return s


def cpp_signature(name, calling_conv='__cdecl'):
    """Generate a stub C++ signature comment."""
    readable = demangle_simple(name)
    return f"// {readable}\nextern \"C\" void* _stub_{name.replace('?', '_')} = 0;\n"


def generate_stubs():
    rows = []
    with open(MANIFEST_IN, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            rows.append(row)

    # Group non-inline functions by (lib, obj_dir)
    # Only process game + engine classes
    groups = defaultdict(list)
    for row in rows:
        cls = row['class']
        if cls not in ('game', 'engine'):
            continue
        if row['is_inline'] == '1':
            continue  # Skip inline/COMDAT

        if cls == 'game':
            obj = row['obj']
            target_dir = GAME_OBJ_DIRS.get(obj, 'game/unknown')
            key = (cls, target_dir)
        else:
            lib = row['lib']
            target_dir = ENGINE_LIB_DIRS.get(lib, f'engine/{lib}')
            key = (cls, target_dir)

        groups[key].append(row)

    # Generate one stub file per (class, dir) pair
    # For game code, combine all into one big stub until PDB phase resolves .cpp mapping
    game_stubs = {}
    engine_stubs = {}

    for (cls, target_dir), funcs in groups.items():
        if cls == 'game':
            if target_dir not in game_stubs:
                game_stubs[target_dir] = []
            game_stubs[target_dir].extend(funcs)
        else:
            if target_dir not in engine_stubs:
                engine_stubs[target_dir] = []
            engine_stubs[target_dir].extend(funcs)

    stub_files = []  # (filepath, func_count)

    # Generate game stubs per subdirectory
    for target_dir, funcs in sorted(game_stubs.items()):
        dir_path = os.path.join(STUBS_DIR, target_dir)
        os.makedirs(dir_path, exist_ok=True)
        stub_path = os.path.join(dir_path, 'stubs_game.cpp')

        with open(stub_path, 'w', encoding='utf-8') as f:
            f.write(f'// AUTO-GENERATED STUBS — {target_dir}\n')
            f.write(f'// {len(funcs)} non-inline functions\n')
            f.write(f'// These will be replaced with real implementations as porting progresses.\n\n')
            f.write('#include <windows.h>\n')
            f.write('#include <cstdio>\n\n')
            f.write('#define COD3_UNIMPLEMENTED(name) do { \\\n')
            f.write('    char _msg[512]; \\\n')
            f.write('    _snprintf(_msg, sizeof(_msg), "COD3 UNIMPLEMENTED: %s\\n", name); \\\n')
            f.write('    OutputDebugStringA(_msg); \\\n')
            f.write('    DebugBreak(); \\\n')
            f.write('} while(0)\n\n')

            sorted_funcs = sorted(funcs, key=lambda r: r['offset'])
            for func in sorted_funcs:
                name = func['name']
                demangled = demangle_simple(name)
                f.write(f'// ea: {func["ida_ea"]}  {demangled}\n')
                f.write(f'// lib:{func["lib"]}  obj:{func["obj"]}\n')
                f.write(f'#pragma comment(linker, "/include:__stub_{name.replace("?", "_")}")\n')
                f.write(f'void __stub_{name.replace("?", "_")}() {{\n')
                f.write(f'    COD3_UNIMPLEMENTED("{name}");\n')
                f.write(f'}}\n\n')

        stub_files.append((stub_path, len(funcs)))
        print(f"  {stub_path}: {len(funcs)} stubs")

    # Generate engine stubs per subdirectory
    for target_dir, funcs in sorted(engine_stubs.items()):
        dir_path = os.path.join(STUBS_DIR, target_dir)
        os.makedirs(dir_path, exist_ok=True)
        stub_path = os.path.join(dir_path, 'stubs.cpp')

        with open(stub_path, 'w', encoding='utf-8') as f:
            f.write(f'// AUTO-GENERATED STUBS — {target_dir}\n')
            f.write(f'// {len(funcs)} non-inline functions\n\n')
            f.write('#include <windows.h>\n')
            f.write('#include <cstdio>\n\n')
            f.write('#define COD3_UNIMPLEMENTED(name) do { \\\n')
            f.write('    OutputDebugStringA("COD3 UNIMPLEMENTED: " name "\\n"); \\\n')
            f.write('    DebugBreak(); \\\n')
            f.write('} while(0)\n\n')

            sorted_funcs = sorted(funcs, key=lambda r: r['offset'])
            for func in sorted_funcs:
                name = func['name']
                demangled = demangle_simple(name)
                f.write(f'// ea: {func["ida_ea"]}  {demangled}\n')
                f.write(f'// lib:{func["lib"]}  obj:{func["obj"]}\n')
                f.write(f'#pragma comment(linker, "/include:__stub_{name.replace("?", "_")}")\n')
                f.write(f'void __stub_{name.replace("?", "_")}() {{\n')
                f.write(f'    COD3_UNIMPLEMENTED("{name}");\n')
                f.write(f'}}\n\n')

        stub_files.append((stub_path, len(funcs)))
        print(f"  {stub_path}: {len(funcs)} stubs")

    total_stubs = sum(n for _, n in stub_files)
    print(f"\nTotal stub files: {len(stub_files)}")
    print(f"Total stubbed functions: {total_stubs}")

    return stub_files


if __name__ == '__main__':
    generate_stubs()
