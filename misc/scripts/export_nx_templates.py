#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

#version_string = version.major + '.' + version.minor + '.' + version.patch + '.' + version.status
version_string = '4.0.4.stable'
appdata_dir = os.getenv('APPDATA')
target_dir = appdata_dir + '\\Godot\\export_templates\\' + version_string + '\\'

print(version_string)
print(appdata_dir)
print(target_dir)

if not os.path.exists(target_dir):
    os.makedirs(target_dir)

# 64 bit debug
if (os.path.isfile('bin\\godot_nx.nx.template_debug.arm64.nss')):
    os.system('copy bin\\godot_nx.nx.template_debug.arm64.nss ' + target_dir)
# 64 bit release
if (os.path.isfile('bin\\godot_nx.nx.template_release.arm64.nss')):
    os.system('copy bin\\godot_nx.nx.template_release.arm64.nss ' + target_dir)
# 32 bit debug
if (os.path.isfile('bin\\godot_nx.nx.template_debug.arm32.nss')):
    os.system('copy bin\\godot_nx.nx.template_debug.arm32.nss ' + target_dir)
# 32 bit release
if (os.path.isfile('bin\\godot_nx.nx.template_release.arm32.nss')):
    os.system('copy bin\\godot_nx.nx.template_release.arm32.nss ' + target_dir)