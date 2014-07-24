def CheckLibCProvidesBDB(context):
	#Taken from man page and modified!
	context.Message( 'Checking whether libc provides Berkeley DB... ' )
	#lastLIBS = context.env['LIBS']
	lastLIBPATH = context.env['LIBPATH']
	lastCPPPATH= context.env['CPPPATH']
	#context.env['LIBS'] = ''
	context.env['LIBPATH'] = ''
	context.env['CPPPATH'] = ''
	ret = context.TryLink("""
	#include "db.h"
	int
	main() {
  	
	return 0;
	}  
	""", '.c')
	
	#This is internal to SCons... not ideal, but provides /* #undef */ if not defined,
	#unlike conf.Define(), which does not provide ANYTHING if not called! Also, _Have
	#adds a newline to make config_h easier to read!
	SCons.Conftest._Have(context, 'LIBC_PROVIDES_LIBDB', ret, 'Set to 1 if the target libc provides a Berkeley DB implementation.')
	
	context.env.Replace(LIBPATH=lastLIBPATH, CPPPATH=lastCPPPATH)
	context.Result( ret )
	return ret
