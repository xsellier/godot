import os
import platform
import sys

def is_active():
    return True


def get_name():
    return "Nintendo Switch"


def can_build():
    return ("NINTENDO_SDK_ROOT" in os.environ)


def get_opts():
    return [
        ('NINTENDO_SDK_ROOT', 'Path to the Nintendo Switch SDK', os.environ.get("NINTENDO_SDK_ROOT", 0)),
    ]


def get_flags():
    return [
            ("module_mobile_vr_enabled", False),
            ('tools', False),
            ('module_mbedtls_enabled', False),
            ('module_upnp_enabled', False),
            ('builtin_pcre2_with_jit', False),
            ('use_volk', False),
            ('builtin_embree', False),
            ('opengl3', False),
            ('arch', 'arm64'),
            ('target', 'template_debug'),
    ]


def configure(env):

    env.msvc = False
    #env.split_modules = True
    env.use_windows_spawn_fix()

    nx_toolchain_path = ""
    nx_toolchain_prefix = ""
    nx_library_path = ""
    is_sdk17 = False

    # common C++ flags
    env.Append(CPPFLAGS=[
        '-fno-common',
        '-fno-short-enums',
        '-ffunction-sections',
        '-fdata-sections',
        '-fPIC',
        '-DNN_NINTENDO_SDK',
        ])
    env.Append(CXXFLAGS=[
        '-std=gnu++17'
    ])
    env.Prepend(CPPPATH=[env["NINTENDO_SDK_ROOT"] + '/Include'])

    # Application link flags
    env.Append(LINKFLAGS=[
        '-nostartfiles',
        '-Wl,--gc-sections', 
        '-Wl,--build-id=md5',
        '-Wl,-init=_init,-fini=_fini',
        '-Wl,-pie',
        '-Wl,-z,combreloc,-z,relro,--enable-new-dtags',
        '-Wl,-u,malloc',
        '-Wl,-u,calloc',
        '-Wl,-u,realloc',
        '-Wl,-u,aligned_alloc',
        '-Wl,-u,free',
    ])

    # shared lib link flags
    env.Append(SHLINKFLAGS=[
        '-nostartfiles',
        '-Wl,--gc-sections', 
        '-Wl,--build-id=md5',
        '-Wl,-init=_init,-fini=_fini',
        '-Wl,--shared',
        '-Wl,--whole-archive',
        '-Wl,--export-dynamic,-z,combreloc,-z,relro,--enable-new-dtags',
    ])

    ## Architecture
    supported_arches = ["arm32", "arm64"]
    if env["arch"] not in supported_arches:
        print("Nintendo Switch: Unsupported architecture: " + env["arch"])
        sys.exit()

    ## check if compiler has moved (SDK 17.1.0+)
    nx_toolchain_path = env["NINTENDO_SDK_ROOT"] + "/Compilers/NintendoClang/bin/"
    is_sdk17 = os.path.exists(nx_toolchain_path)
    ld_command = nx_toolchain_path + 'ld.lld.exe'

    # aarch64
    if (env["arch"] == "arm64"):
        if not is_sdk17:
            nx_toolchain_path = env["NINTENDO_SDK_ROOT"] + "/Compilers/NX/nx/aarch64/bin/"
            nx_toolchain_prefix = "aarch64-nintendo-nx-elf-"
            ld_command = nx_toolchain_path + nx_toolchain_prefix + 'ld.gold.exe'
        
        env.Append(CPPFLAGS=[
            '-mcpu=cortex-a57+fp+simd+crypto+crc',
        ])
        if is_sdk17:
            env.Append(CPPFLAGS=[
                '--target=aarch64-nintendo-nx-elf'
            ])

        env.Prepend(CPPPATH=[env["NINTENDO_SDK_ROOT"] + '/TargetSpecificInclude/NX-NXFP2-a64/'])
        
        # aarch64 specific shared lib link flags
        env.Append(SHLINKFLAGS=[
            '-mcpu=cortex-a57+fp+simd+crypto+crc',
            '-Wl,-T',
            env["NINTENDO_SDK_ROOT"] + "/Resources/SpecFiles/RoModule.aarch64.lp64.ldscript",
            '-fuse-ld=' + ld_command,
        ])

        # aarch64 specific application link flags
        env.Append(LINKFLAGS=[
            '-mcpu=cortex-a57+fp+simd+crypto+crc',
            '-Wl,-T',
            env["NINTENDO_SDK_ROOT"] + "/Resources/SpecFiles/Application.aarch64.lp64.ldscript",
            '-fuse-ld=' + ld_command,
        ])

        if is_sdk17:
            env.Append(SHLINKFLAGS=[
                '--target=aarch64-nintendo-nx-elf'
            ])
            env.Append(LINKFLAGS=[
                '--target=aarch64-nintendo-nx-elf'
            ])

        if (env["target"] == "template_release"):
            # release
            nx_library_path = env["NINTENDO_SDK_ROOT"] + "/Libraries/NX-NXFP2-a64/Release"
        else:
            # debug
            nx_library_path = env["NINTENDO_SDK_ROOT"] + "/Libraries/NX-NXFP2-a64/Develop"

    # armv7l
    if (env["arch"] == "arm32"):
        if not is_sdk17:
            nx_toolchain_path = env["NINTENDO_SDK_ROOT"] + "/Compilers/NX/nx/armv7l/bin/"
            nx_toolchain_prefix = "armv7l-nintendo-nx-eabihf-"
            ld_command = nx_toolchain_path + nx_toolchain_prefix + 'ld.gold.exe'

        env.Append(CPPFLAGS=[
            '-mabi=aapcs-linux',
            '-mcpu=cortex-a57',
            '-mfpu=crypto-neon-fp-armv8',
            '-mfloat-abi=hard',
        ])
        if is_sdk17:
            env.Append(CPPFLAGS=[
                '--target=armv7l-nintendo-nx-eabihf'
            ])

        env.Prepend(CPPPATH=[env["NINTENDO_SDK_ROOT"] + '/TargetSpecificInclude/NX-NXFP2-a32/'])

        # arm specific shared lib link flags
        env.Append(SHLINKFLAGS=[
            '-mabi=aapcs-linux',
            '-mcpu=cortex-a57',
            '-mfloat-abi=hard',
            '-mfpu=crypto-neon-fp-armv8',
            '-Wl,--target2=got-rel',
            '-Wl,-T',
            env["NINTENDO_SDK_ROOT"] + "/Resources/SpecFiles/RoModule.arm.ilp32.ldscript",
            '-fuse-ld=' + ld_command,
        ])

        # arm specific applicaiton link flags
        env.Append(LINKFLAGS=[
            '-mabi=aapcs-linux',
            '-mcpu=cortex-a57',
            '-mfloat-abi=hard',
            '-mfpu=crypto-neon-fp-armv8',
            '-Wl,--target2=got-rel',
            '-Wl,-T',
            env["NINTENDO_SDK_ROOT"] + "/Resources/SpecFiles/Application.arm.ilp32.ldscript",
            '-fuse-ld=' + ld_command,
        ])

        if is_sdk17:
            env.Append(SHLINKFLAGS=[
                '--target=armv7l-nintendo-nx-eabihf'
            ])
            env.Append(LINKFLAGS=[
                '--target=armv7l-nintendo-nx-eabihf'
            ])
        
        if (env["target"] == "template_release"):
            # release
            nx_library_path = env["NINTENDO_SDK_ROOT"] + "/Libraries/NX-NXFP2-a32/Release"
        else:
            # debug
            nx_library_path = env["NINTENDO_SDK_ROOT"] + "/Libraries/NX-NXFP2-a32/Develop"


    ## Build type

    if (env["target"] == "template_release"):
        env.Append(CPPFLAGS=[
            '-O3',
            '-fomit-frame-pointer',
            '-DNN_SDK_BUILD_RELEASE',
            ])

    elif (env["target"] == "template_debug"):
        env.Append(CPPFLAGS=[
            '-g',
            # '-O0',
            '-O3',
            '-fno-omit-frame-pointer',
            '-DNN_SDK_BUILD_DEBUG', 
            '-DDEBUG_ENABLED', 
            '-DDEBUG_MEMORY_ENABLED',
            ])
    
    ## Compiler configuration

    if is_sdk17:
        # now nx_toolchain_prefix should be set
        nx_toolchain_prefix = 'llvm-'

    env['CC'] = nx_toolchain_path + 'clang'
    env['CXX'] = nx_toolchain_path + 'clang++'
    env['AR'] = nx_toolchain_path + nx_toolchain_prefix + 'ar'
    env['AS'] = nx_toolchain_path + nx_toolchain_prefix + 'as'
    env['LINK'] = nx_toolchain_path + 'clang++'
    env['RANLIB'] = nx_toolchain_path + nx_toolchain_prefix + 'ranlib'

    ## special nx magic
    nx_c_runtime_ro = nx_library_path + "/rocrt.o"
    nx_c_runtime_nro = nx_library_path + "/rocrt_nro.o"
    nx_c_runtime_ro_end = nx_library_path + "/crtend.o"
    nx_nn_application = nx_library_path + "/nnApplication.o" + " " + nx_library_path + "/libnn_init_memory.a"
    
    nx_nn_sdk_lib = nx_library_path + "/nnSdk.nss"
    nx_opengl_lib = nx_library_path + "/opengl.nss"
    nx_vulkan_lib = nx_library_path + "/vulkan.nss"
    nx_gll_lib = nx_library_path + "/libnn_gll.a"

    nx_link_prefix = [
        '-Wl,--start-group',
        nx_c_runtime_ro,
        nx_nn_application,
    ]

    nx_link_suffix = [
        # static libs here..
        nx_gll_lib, # needed to dynamicly load GL functions
        '-Wl,--end-group',
        # #include *.nrs dynamic libs here
        '-Wl,--start-group',
        nx_opengl_lib, # will always need for EGL/OpenGLES
        nx_vulkan_lib,
        nx_nn_sdk_lib,
        '-Wl,--end-group',
        nx_c_runtime_ro_end,
    ]

    nx_shlink_prefix = [
        '-Wl,--start-group',
        nx_c_runtime_nro,
    ]

    nx_shlink_suffix = [
        '-Wl,--end-group',
        nx_c_runtime_ro_end,
    ]

    # define custom application link command
    env['PROGSUFFIX'] = ".nss"
    env['LINKCOM'] = "$LINK $LINKFLAGS " + ' '.join(nx_link_prefix) + " $_LIBDIRFLAGS $_LIBFLAGS $SOURCES " + ' '.join(nx_link_suffix) + " -o $TARGET"

    # define custom shared library link command
    env['SHLIBSUFFIX'] = ".nrs"
    env['SHLINKCOM'] = "$LINK $SHLINKFLAGS " + ' '.join(nx_shlink_prefix) + " $_LIBDIRFLAGS $_LIBFLAGS $SOURCES " + ' '.join(nx_shlink_suffix) + " -o $TARGET"

    # Shared Object file ending: .nrs
    # Dynamic Module: *.nrs -> MakeNro -> *.nro

    # Application binaries (static library): .nss
    # Runnable binary: *.nss -> MakeNso -> *.nso

    # Packaging for Deployment
    # .nmeta -> MakeMeta -> .npdm
    # (optionaly) .nro -> MakeNrr -> .nrr
    # .npdm, .nso, (.nrr) -> AuthoringTool -> .nsp
    # *.nsp files are what is deployed to hardware


    ## flags
    env.Append(CPPPATH=['#platform/nx'])
    env.Append(CPPDEFINES=[
        'NX_ENABLED',
        'UNIX_SOCKET_UNAVAILABLE',
        'VULKAN_ENABLED',
        ])