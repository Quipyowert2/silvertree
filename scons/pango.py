# vi: syntax=python:et:ts=4
import os
from os.path import join

def CheckPango(context, backend):
    context.Message("Checking for Pango with " + backend + " backend... ")
    env = context.env
    gtkdir = os.environ.get("GTK_BASEPATH")
    if gtkdir:
        env.AppendENVPath("PATH", join(gtkdir, "bin"))
        env.AppendENVPath("PKG_CONFIG_PATH", join(gtkdir, "lib/pkgconfig"))
    result, output = context.TryAction("pkg-config --libs --cflags pango" + backend + " > $TARGET")
    if result:
        env.MergeFlags(output)
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

def get_checks():
    return { "CheckPango" : CheckPango }