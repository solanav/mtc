from distutils.core import setup, Extension

module1 = Extension('mtc',
                    sources=['mtc.c'],
                    extra_compile_args=['-O0'],
                    libraries=['pthread'])

setup(name='mtc',
      version='0.0.1',
      description='Threading module',
      ext_modules=[module1])
