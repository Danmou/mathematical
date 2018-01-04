from distutils.core import setup, Extension
import pkgconfig
import subprocess
from pathlib import Path
import shutil

mtexbuildpath = Path('ext/mathematical/mtex2MML/build')
# shutil.rmtree(mtexbuildpath, ignore_errors=True)
# mtexbuildpath.mkdir()
# subprocess.call(['cmake', '..'], cwd=mtexbuildpath)
# subprocess.call(['make', 'libmtex2MML_static'], cwd=mtexbuildpath)

lasembuildpath = Path('ext/mathematical/lasem/build')
# shutil.rmtree(lasembuildpath, ignore_errors=True)
# lasembuildpath.mkdir()
# subprocess.call(['cmake', '../..'], cwd=lasembuildpath)
# subprocess.call(['make'], cwd=lasembuildpath)

incdirs = pkgconfig.parse('glib-2.0 gdk-pixbuf-2.0 cairo pango --static')['include_dirs']
module = Extension("process",
                   include_dirs = ['ext/mathematical', 'ext/mathematical/mtex2MML/src', 'ext/mathematical/lasem/src'] + incdirs,
                   libraries = ['lasem', 'mtex2MML'],
                   library_dirs = [str(mtexbuildpath), str(lasembuildpath)],
                   # extra_compile_args=['-w'],
                   # extra_compile_args=['-Wl,--no-undefined'],
                   # extra_link_args=['-static'],
                   # sources = ["ext/mathematical/process.cpp"]
                   sources = ["ext/mathematical/cairo_callbacks.c", "ext/mathematical/lasem_overrides.c", "ext/mathematical/process.cpp"]
                   )
# module2 = Extension("cairo_callbacks",
#                    include_dirs = ['ext/mathematical', 'ext/mathematical/mtex2MML/src', 'ext/mathematical/lasem/src'] + incdirs,
#                    libraries = ['lasem', 'mtex2MML'],
#                    library_dirs = [str(mtexbuildpath), str(lasembuildpath)],
#                    sources = ["ext/mathematical/cairo_callbacks.c"]
#                    )
# module3 = Extension("lasem_overrides",
#                    include_dirs = ['ext/mathematical', 'ext/mathematical/mtex2MML/src', 'ext/mathematical/lasem/src'] + incdirs,
#                    libraries = ['lasem', 'mtex2MML'],
#                    library_dirs = [str(mtexbuildpath), str(lasembuildpath)],
#                    sources = ["ext/mathematical/lasem_overrides.c"]
#                    )

setup(name = "PyMathematical", data_files = [('config', ['setup.cfg'])], ext_modules=[module])