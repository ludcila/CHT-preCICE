# encoding: utf-8

"""
WAF configuration file for Ubuntu and specific Code_Aster versions.
"""


def configure(self):
    # from Options import options as opts
    opts = self.options
    self.env.append_value('LIBPATH', [
        '/srvtc/cloud/aster-12.6.0/build/public/hdf5-1.8.14/lib',
        '/srvtc/cloud/aster-12.6.0/build/public/med-3.2.0/lib',
        '/srvtc/cloud/aster-12.6.0/build/public/metis-4.0.3/lib',
        '/srvtc/cloud/aster-12.6.0/build/public/scotch-5.1.11/lib',
        '/srvtc/cloud/aster-12.6.0/contrib/OpenBLAS-0.2.18/build/lib'])

    self.env.append_value('INCLUDES', [
        '/srvtc/cloud/aster-12.6.0/build/public/hdf5-1.8.14/include',
        '/srvtc/cloud/aster-12.6.0/build/public/med-3.2.0/include',
        '/srvtc/cloud/aster-12.6.0/build/public/metis-4.0.3/include',
        '/srvtc/cloud/aster-12.6.0/build/public/scotch-5.1.11/include'])

    opts.maths_libs = 'openblas'
    opts.embed_math = True

    opts.enable_med = True
    opts.med_libs = 'med stdc++'
    opts.embed_med = True

    opts.hdf5_libs = 'hdf5 z'
    opts.embed_hdf5 = True

    opts.enable_scotch = True
    opts.embed_scotch = True

    opts.embed_aster = True
    opts.embed_fermetur = True

    opts.enable_mumps = True

    opts.enable_metis = True
    opts.enable_petsc = False
