def SetBuildEnvVars(env):
	if env['OS_ENV']:
		env['ENV'] = os.environ

	if 'INCLUDE_PATH' in env:
		env.Prepend(CPPPATH= env['INCLUDE_PATH'])
	if 'LIBRARY_PATH' in env:
		env.Prepend(LIBPATH= env['LIBRARY_PATH'])
	if 'RUNTIME_PATH' in env:
		env.Prepend(RPATH= env['RUNTIME_PATH'])
	
	#env['CFLAGS'] = '-ansi -Wall -Wextra' #libdb configure test will fail.
	env['CFLAGS'] = '-Wall -Wextra'
	
	if env['DEBUG']:
		env.Append(CFLAGS = ' -pedantic -O0 -gdwarf-2 -Wno-long-long -Wno-unused-parameter'\
			' -Wno-unused-but-set-parameter')
		env.Append(LINKFLAGS = ' -gdwarf-2') 
		env.Append(CPPDEFINES = []) 
	else:
		env.Append(CFLAGS = ' -pedantic -O2')
		env.Append(CPPDEFINES = ['NDEBUG', 'NDEBUG_MESSAGES', 'NPRINT_OUTPUT']) 
	env.Append(CPPPATH = [Dir('#/include')])



def RunConfigureTests(env):
	if not env.GetOption('clean') and not env.GetOption('help'):
		conf = Configure(env, config_h=File('include/cc_config.h'), custom_tests = \
		{ 'CheckLibCProvidesBDB' : CheckLibCProvidesBDB }) #, clean=False, help=False)- Don't work...
			
		if not conf.CheckCC():
			print 'Error: C compiler does not work!'
			Exit(1)
			
		for posix_header in ['sys/types.h', 'sys/socket.h', 'netinet/in.h', 'arpa/inet.h', \
		'netdb.h', 'unistd.h']:
			if not conf.CheckHeader(posix_header, include_quotes='<>', language='C'):
				print 'Error: POSIX header {0} was not found in the target headers.'.format(posix_header)
				Exit(1)
				
		if conf.CheckLibCProvidesBDB(): #It is possible to cause a mismatch between header and library if libc provides BDB.
			#This step is a precaution.
			pass
			#http://www.scons.org/doc/production/HTML/scons-api/SCons.SConf-pysrc.html#CheckLib- bug?
			#conf.Define('LIBC_PROVIDES_LIBDB', 1, 'Set to 1 if the target libc provides a Berkeley DB implementation.')
			
		#This test causes the following failure with distcc:
		#distccd[4089] (dcc_r_token_int) ERROR: read failed while waiting for token "DOTI"	
		for db_lib, db_header in zip(['db5'], \
			['db5/db.h']):
			if conf.CheckLibWithHeader(db_lib, db_header, 'C', autoadd=True):
				break
		else:
			print 'Error: Berkeley DB and/or its headers were not found in the target library paths.'
			Exit(1)
		
		if not conf.CheckLibWithHeader('jansson', 'jansson.h', 'C', autoadd=True):
			print 'Error: Jansson and/or its headers were not found in the target library paths.'
			Exit(1)
		
		conf.Finish()


def CheckLibCProvidesBDB(context):
	#Taken from man page and modified!
	context.Message( 'Checking whether libc provides Berkeley DB... ' )
	lastLIBS = context.env.get('LIBS', '')
	lastLIBPATH = context.env.get('LIBPATH', '')
	lastCPPPATH= context.env.get('CPPPATH', '')
	
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
