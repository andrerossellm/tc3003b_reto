# -*- mode: python ; coding: utf-8 -*-

from pathlib import Path

a = Analysis(
    ['gui.py'],
    pathex=[],
    binaries=[],
    datas=[('images/prices.png', 'images'), ('totalLocalitiesScanned.txt', '.'), ('intel.out', '.')],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='Process Images',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
app = BUNDLE(
    exe,
    name='processImages.app',
    icon=None,
    bundle_identifier=None,
)
