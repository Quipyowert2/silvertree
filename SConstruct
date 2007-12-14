# vi: syntax=python:et:ts=4

from os.path import join
from glob import glob

opts = Options("options.cache")
opts.AddOptions(
    ("PREFIX", "Install prefix.", "/usr/local"),
    ("QTDIR", "Root directory of Qt's installation.", "/usr/"),
    EnumOption("GL_IMPL_MAC", "Mac-specific: OpenGL implementation to be used: Mesa 3D or Apple.", "mesa", ["mesa", "apple"], ignorecase=1),
    ("SDLDIR", "Root directory of SDL's installation.", "/usr/"),
    ("BOOSTDIR", "Root directory of boost installation.", "/usr/include"),
    ("BOOSTLIBS", "Directory where boost libs are located.", "/usr/lib"),
    ("BOOST_SUFFIX", "Suffix of the boost regex library.", ""),
    EnumOption("Build", "Build variant: debug or release", "release", ["debug", "release"], ignorecase=1),
    ("EXTRA_FLAGS_RELEASE", "Extra compiler/linker flags to use in release build variant.(e.g. \"-O3 -march=prescott\")", ""),
    ("EXTRA_FLAGS_DEBUG", "Extra compiler/linker flags to use in debug build variant.", "")
)

env = Environment(tools = ["zip"], toolpath = ["scons"], options = opts)
if env["PLATFORM"] == "win32":
    env.Tool("mingw")
else:
    env.Tool("default")
opts.Save("options.cache", env)

Help("""
Type "scons" to build the game.
"scons editor"         - builds the editor.
"scons namegen"        - namegen.
"scons version_finder" - version finder.

Additional options:
""")
Help(opts.GenerateHelpText(env))

print "Configuring for " + env["PLATFORM"] + " platform..."
if env["PLATFORM"] == "darwin":
    env.AppendUnique(LIBPATH = ["/sw/lib"])
    if env["GL_IMPL_MAC"] == "mesa":
        env.AppendUnique(LIBPATH = ["/sw/lib/mesa"], CPPPATH = ["/sw/include/mesa"])
    if env["GL_IMPL_MAC"] == "apple":
        env.AppendUnique(FRAMEWORKS = ["OpenGL"], CPPPATH = ["/System/Library/Frameworks/OpenGL.framework/Headers"])
env.AppendUnique(CPPPATH = [env["BOOSTDIR"]], LIBPATH = [env["BOOSTLIBS"]])

execfile("SConfigure")
conf = env.Configure(custom_tests = CustomChecks)
conf.CheckBoost("regex", "1.20") or Exit(1)
conf.Finish()
namegen_env = env.Clone()
conf = env.Configure(custom_tests = CustomChecks)
conf.CheckOpenGL(["gl", "glu"]) and \
conf.CheckSDL(require_version = "1.2.10") and \
conf.CheckSDL("SDL_image") and \
conf.CheckSDL("SDL_ttf") or Exit(1)
conf.Finish()

editor_env = env.Clone()
try:
    editor_env.Tool("qt4")
    editor_env.EnableQt4Modules(["QtCore", "QtGui", "QtOpenGL"])
    editor_env.Replace(QT4_UICIMPLSUFFIX=".hpp", QT4_UICDECLSUFFIX=".hpp",
        QT4_UICIMPLPREFIX="", QT4_UICDECLPREFIX="", QT4_MOCHPREFIX="", QT4_MOCHSUFFIX=".moc",
        QT4_AUTOSCAN=0)
except:
    pass
editor_conf = editor_env.Configure()
if env["PLATFORM"] == "win32":
    QtLibSuffix = "4"
else:
    QtLibSuffix = ""
editor_env["HaveQt"] = editor_conf.CheckLibWithHeader("QtCore" + QtLibSuffix, "QtGlobal", "C++", autoadd = False) and \
         editor_conf.CheckLibWithHeader("QtGui" + QtLibSuffix, "QApplication", "C++", autoadd = False) and \
         editor_conf.CheckLibWithHeader("QtOpenGL" + QtLibSuffix, "QGLWidget", "C++", autoadd = False)
editor_conf.Finish()

if "gcc" in env["TOOLS"]:
    ccflags = Split("-Wall -Wno-sign-compare -Wno-switch -Wno-switch-enum")
    linkflags = []
    if env["Build"] == "release":
        ccflags.append("-O2")
        linkflags.append("-s")
    if env["Build"] == "debug":
        ccflags.append("-ggdb")
    env.AppendUnique(CCFLAGS = ccflags, LINKFLAGS = linkflags)
    editor_env.AppendUnique(CCFLAGS = ccflags, LINKFLAGS = linkflags)

env.MergeFlags(env["EXTRA_FLAGS_" + env["Build"].upper()], unique=0)
editor_env.MergeFlags(env["EXTRA_FLAGS_" + env["Build"].upper()], unique=0)

namegen = namegen_env.Program("utilities/names/namegen", "utilities/names/namegen.cpp")
Alias("namegen", namegen)

env.Program("version_finder", "utilities/versions/version_finder.cpp")

Export("env")
Export("editor_env")
silvertree, editor = SConscript("src/SConscript", build_dir = join("build", env["Build"]))

if env["Build"] == "debug":
    ExecutableSuffix = "-debug" + env["PROGSUFFIX"]
else:
    ExecutableSuffix = env["PROGSUFFIX"]
env.Default(env.Alias("silvertreerpg", env.InstallAs("./silvertreerpg" + ExecutableSuffix, silvertree)))
if editor:
    editor_env.Alias("editor", editor_env.InstallAs("./silvertreeedit" + ExecutableSuffix, editor))

executables = [silvertree]
if editor_env["HaveQt"]:
    executables.append(editor)
datafiles = Split("data images model-sources FreeSans.ttf") + map(glob, Split("*.dae *.3ds *.3d"))

data_install_dir = join(env["PREFIX"], "share/silvertree-rpg")
install_executables = Install(join(env["PREFIX"], "bin"), executables)
install_data = Install(data_install_dir, datafiles)
if not env["PLATFORM"] == "win32":
    Alias("install", [install_executables, install_data])
    env.Replace(CPPDEFINES = { "HAVE_CONFIG_H" : None, "DATADIR" : '\\"' + data_install_dir + '\\"' })
    editor_env.Replace(CPPDEFINES = { "HAVE_CONFIG_H" : None, "DATADIR" : '\\"' + data_install_dir + '\\"' })

bindistdir = env.Install("silvertree", Split("README LICENSE") + executables + datafiles)
bindistzip = env.Zip("silvertree.zip", bindistdir)
env.Alias("bindistzip", bindistzip)

SConsignFile("sconsign")
