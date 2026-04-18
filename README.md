# Godot Engine for Nintendo Switch

<p align="center">
  <a href="https://godotengine.org">
    <img src="logo_outlined.svg" width="400" alt="Godot Engine logo">
  </a>
</p>

## Build Instructions

First make sure you have a valid Nintendo SDK (at least version 15) install, which should also add the environment variable ```NINTENDO_SDK_ROOT```. Make sure that is set because it is required both for building Godot as well as when exporting from the editor. This instruction guide also assumes you have a recent MSVC (tested with 2022) environment and a console already configured for its use.

Multiple builds must be done before Godot can be used for Nintendo Switch development.  Clone this repo, change into the directory and build the following configurations:

```shell
scons platform=windows target=editor

scons platform=nx target=template_release arch=arm64
scons platform=nx target=template_debug debug_symbols=yes arch=arm64

scons platform=nx target=template_release arch=arm32
scons platform=nx target=template_debug debug_symbols=yes arch=arm32
```


You don't necessarily need to build for all 4 device configurations but these are all the potential export templates.  The Windows build provides the editor and exporter for the NX.

Once the export templates are generated you need to deploy your local export templates folder. There is a scripted provided to do this for you:

```python misc/scripts/export_nx_templates.py```

If you make any changes to this project you need to build and redeploy the export templates.

### Build Flags
Certain features are only available once the editor and export templates have been built with additional build flags.

| Flag                            | Description                                                                                                   |
|---------------------------------|---------------------------------------------------------------------------------------------------------------|
| ``nx_use_preselected_user=yes`` | If your game does not require changing user profiles while in-game, this flag may help avoid lotcheck issues. |
| ``nx_web_module=yes``           | This flag enables GDScript functionality for the nn::web api to display web content.                          |

## Export and Deployment

Run the Godot editor project you built for Windows.  

To export an ```.nsp``` file for your project open the export dialog from the file menu:

```Project->Exports...```

Click the ```Add...``` button and Select ```Nintendo Switch```.  Now click on ```Nintendo Switch``` which enables you to apply your export settings.

Export settings for Nintendo Switch uses the `PublishingMetadataNX` resource. It is recommended that you do *not* save the resource as a file. (leave it embedded in the export preset)

Options in `PublishingMetadataNX` correspond to NMETA entries. Please refer to the NintendoSDK documents (Features > Building > Creating Application Images > Writing the NMETA File) for more information. You must configure at least one localization (AmericanEnglish for example) and set the name and icon there.

You don't need to fill every field in `PublishingMetadataNX` in order to export successfully. Just remember to fill out the rest when your game's ready for lotcheck.

Icons must be 1024x1024 in size, and be either a JPEG or 24-bit BMP file.  This is very important because ```.nsp``` generation will fail if the Icon file is not one of these very specific formats!

In case you get a '[Error] (Issue 10-820) Big icon format is not baseline JPEG' error even if everything is correct, you need to exclude the actual icon file from Godot's imports.


Now click ```Export Project``` to generate and save an ```.nsp``` file. This is the final file you install to your development hardware.  It's also possible to debug your game by opening and launching this ```.nsp``` file in Visual Studio.

It is also possible to launch your Game directly from Godot provided you have already set valid export settings. To do this click the Nintendo Switch icon in the top right corner of the editor.

## Important Notes for Exporting to Nintendo Switch

Please use Nintendo SDK version 19.x.

Currently, Godot NX 4.x only supports Vulkan. To ensure proper functionality, follow these steps:

1. Go to ```Project->Project Setting->Renderer...```
2. Set Rendering Method to ```mobile```
3. Set Rendering Method.mobile to ```mobile```
4. Set Rendering Method.web to ```gl_compatibility```

Important: If you set Rendering Method/Rendering Method.mobile to ```gl_compatibility```, the exported file will crash when executed.

## Debugging

### C/C++

To debug your game and the Godot engine itself, open the ```.nsp``` file you exported in Visual Studio.  Launch this file in debug mode, and associate Visual Studio with the Godot source code directory when necessary.

### Godot Remote Debugging/Profiling

It is possible to remotely debug your game running on the development hardware via the network. Make sure that you have a local network connection configured on your development hardware, then configure the Godot Editor with the expected IP address of your device like you would when remote debugging in Godot.  The steps necessary in the editor to achieve this are to first enable ```Deploy with Remote debug``` in the ```Debug``` menu. Then in the ```Editor Settings...``` dialog accessed through the ```Editor``` menu, In the ```Network->Debug``` setting set the remote host to be your local network ip address.  The default is ```127.0.0.1``` which will not work because you are not running on the same machine. Instead it needs to be an IP address that is accessible on the same network you have connected your development hardware to.

When the game is launched in remote debugging mode, Godot should connect to the device and allow you to break and step through Godot Script code, As well as profile and monitor the contents performance on the hardware.  In addition you should be able to remotely modify your scene on the device by changing properties in the Remote scene editor.

### RenderDoc

RenderDoc is included in the Nintendo SDK. To be able to attach to a running game for capturing frames, Enable the Renderdoc checkbox in the debug section of the Nintendo Switch Export options. When this checkbox is enabled, your game will be linked against the modified OpenGL libraries necessary to capture with RenderDoc. When it's working you will see additional text at the top left for your game window.

Once this is working, you can connect to the game session running on the hardware by running the version of RenderDoc that comes with the Nintendo SDK. Then you just need to capture a frame from the RenderDoc interface to inspect the state of the graphics pipeline when rendering the captured frame. For more details consult the Nintendo SDK documentation on RenderDoc.

## Profiling
By adding `use_profiler=true` to the build instruction, you can enable per-platform profiler integration. Only the Switch profiler (Nintendo CPU Profiler) is implemented at the moment. You can find the Nintendo CPU Profiler executable in the `<NINTENDO_SDK_ROOT>/Tools/NintendoCpuProfiler` directory.

By default, this will enable per-frame markers (called heartbeats) in the profiler. You can further configure the integration with the following options.
* `profile_callables` (default: `true`)
  * Hint to tell the current profiler (if enabled) if function calls made via ClassDB (GDScript methods, and calls to methods bound via ClassDB::bind_method) should be profiled.
  * Has some noticeable overhead, but still faster than using Godot's remote debugger.
  * The default implementation uses the function name as the identifier. You can modify the source code to add more information (like prepending the script instance's name), but keep in mind this will only add to the profiling overhead. Plus, you can usually guess which script is being referenced by viewing the call stack in the profiler.
* `profile_signals` (default: `false`)
  * Hint to tell the current profiler (if enabled) if signals should be profiled.
  * The default implementation only uses the signal name as the identifier. You can consider adding additional information like the number of listeners, etc, but again this means more overhead.
* `profile_resource_load` (default: `true`)
  * Hint to tell the current profiler (if enabled) if ResourceLoader::load should be profiled for each resource.
  * Very useful for understanding load time bottlenecks. Usual culprits are large binaries like PNGs and WAVs.

Note that Godot's threads can migrate, therefore in-process profiling won't be accurate (the profiler GUI will warn you). Remember to switch to **out-of-process profiling** before starting to sample from the profiler.
