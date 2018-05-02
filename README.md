# DXUP
## A D3D10 -> D3D11 Translation Layer

### What's the point?
 - The main reason is for [DXVK](https://github.com/doitsujin/dxvk), a d3d11->Vulkan translation layer. I'm sure where you can see where this is going...
 - You can use this if you're a lazy gamedev and want to get access to some d3d11 features by querying the interfaces.

### How do I use it?
Simple (Linux Only):
Use ``./package-release.sh master /your/target/directory --no-package`` to automagically.
Advanced (Windows & Linux):
Use meson to build the dlls.

Then set those up as wine overrides if you're on Linux or if you're on windows copy them & the d3d10_original and dxgi_original dlls for the right arch to the game/application folder.
If you wish to use DXVK then use their dxgi as the dxgi_original dll and the DXUP one as the main override.

### Screenshots

DXVK:
[Crysis Warhead Menu](https://i.imgur.com/q1l2gLb.png)
[Crysis Warhead Game](https://i.imgur.com/7yY5bZy.jpg)

Native:
[Just Cause 2 Benchmark](https://cdn.discordapp.com/attachments/438131362390867979/439554845616242688/unknown.png)

# Have fun!
