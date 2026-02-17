#!/usr/bin/env python3
# vim: set ts=8 sts=4 sw=4 tw=99 et:
#
# CS2-Kit - Reusable C++23 library for CS2 Metamod:Source plugins
# AMBuild Configuration Script

import os
import sys

# Add AMBuild path from vendor/mmsource-2.0 if available
mms_ambuild = os.path.join(os.path.dirname(__file__), "vendor", "mmsource-2.0", "third_party", "ambuild")
if os.path.exists(mms_ambuild):
    sys.path.insert(0, mms_ambuild)

from ambuild2 import run  # noqa: E402

parser = run.BuildParser(sourcePath=sys.path[0], api="2.2")

# SDK options
parser.options.add_argument(
    "--hl2sdk-root",
    type=str,
    dest="hl2sdk_root",
    default=None,
    help="Root path containing HL2SDK directories",
)

parser.options.add_argument(
    "--hl2sdk-manifests",
    type=str,
    dest="hl2sdk_manifests",
    default=None,
    help="Path to hl2sdk-manifests",
)

parser.options.add_argument(
    "--mms-path",
    type=str,
    dest="mms_path",
    default=None,
    help="Path to Metamod:Source 2.0",
)

parser.options.add_argument(
    "--sdks",
    type=str,
    dest="sdks",
    default="cs2",
    help="SDK(s) to build (comma-separated, e.g., 'cs2')",
)

parser.options.add_argument(
    "--targets",
    type=str,
    dest="targets",
    default=None,
    help="Target architectures (comma-separated, e.g., 'x86_64')",
)

# Build options
parser.options.add_argument(
    "--enable-debug",
    action="store_const",
    const="1",
    dest="debug",
    default="0",
    help="Enable debugging symbols",
)

parser.options.add_argument(
    "--enable-optimize",
    action="store_const",
    const="1",
    dest="opt",
    default="1",
    help="Enable optimization",
)

parser.Configure()
