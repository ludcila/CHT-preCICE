# encoding: utf-8

"""
WAF MPI configuration file for Ubuntu and specific Code_Aster versions.
"""

import Ubuntu_gnu


def configure(self):
    # from Options import options as opts
    opts = self.options
    Ubuntu_gnu.configure(self)
    self.env['ADDMEM'] = 400

    self.env.prepend_value('LIBPATH', [
        '/srvtc/cloud/aster-12.6.0/contrib/mumps-4.10.0_mpi/lib',
        '/srvtc/cloud/aster-12.6.0/contrib/petsc-3.4.5/arch-linux2-c-opt/lib',
        '/srvtc/cloud/aster-12.6.0/contrib/scalapack-1.0.2/build/lib'])

    self.env.prepend_value('INCLUDES', [
    	'/srvtc/cloud/aster-12.6.0/contrib/mumps-4.10.0_mpi/include',
        '/srvtc/cloud/aster-12.6.0/contrib/petsc-3.4.5/arch-linux2-c-opt/include',
        '/srvtc/cloud/aster-12.6.0/contrib/petsc-3.4.5/include'])

    self.env.append_value('LIB', ('X11',))

    self.env.prepend_value('PYTHONPATH', ['/srvtc/cloud/SimScaleCloudTools/src/contribCodeAster/'])

    opts.parallel = True

    opts.enable_mumps = True
    opts.mumps_version = '4.10.0'
    opts.mumps_libs = 'dmumps zmumps smumps cmumps mumps_common pord metis scalapack openblas esmumps scotch scotcherr'

    opts.enable_petsc = True
    opts.petsc_libs = 'petsc HYPRE ml'
    opts.embed_petsc = True
