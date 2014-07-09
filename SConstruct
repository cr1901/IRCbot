import os

dummy_env = DefaultEnvironment()

vars = Variables()
vars.Add(BoolVariable('DEBUG', 'Set to 1 to set Debug options.', 0))
vars.Add(BoolVariable('OS_ENV', 'Allow SCons to use the System Path and OS Environment (useful for distcc)', 0))
vars.Add('CC', 'Choose your C Compiler. Defaults to DefaultEnvironment()[''CC''].', dummy_env['CC'])
#vars.Add('CXX', 'Choose your C++ Compiler. Defaults to DefaultEnvironment()[''CXX''].', dummy_env['CXX'])

env = Environment(variables = vars) 
Help(vars.GenerateHelpText(env))

if env['OS_ENV']:
	env['ENV'] = os.environ

#env['CFLAGS'] = '-ansi -Wall -Wextra' #libdb configure test will fail.
env['CFLAGS'] = '-Wall -Wextra'

if ARGUMENTS.get('DEBUG', False):
	env.Append(CFLAGS = ' -pedantic -O0 -gdwarf-2 -Wno-unused-parameter'\
		' -Wno-unused-but-set-parameter')
	env.Append(LINKFLAGS = ' -gdwarf-2') 
else:
	env.Append(CFLAGS = ' -pedantic-errors -O2')
	env.Append(CPPDEFINES = ['NDEBUG', 'NDEBUG_MESSAGES', 'NPRINT_OUTPUT']) 
	
#conf = Configure(env, config_h='cc_config.h', clean=False, help=False) #Appears to be broken...
if not env.GetOption('clean') and not env.GetOption('help'):
	conf = Configure(env) #, clean=False, help=False)- Don't work...
	if not conf.CheckCC():
		print 'Error: C compiler does not work!'
		Exit(1)
	for posix_header in ['sys/types.h', 'sys/socket.h', 'netinet/in.h', 'arpa/inet.h', \
	'netdb.h', 'unistd.h']:
		if not conf.CheckHeader(posix_header, include_quotes='<>', language='C'):
			print 'Error: POSIX header {0} was not found in the target headers.'.format(posix_header)
			Exit(1)
	
	for lib_header, lib in zip(['db.h', 'jansson.h'], ['db', 'jansson']):
		if not conf.CheckLibWithHeader(lib, lib_header, 'C', autoadd=True):
			print 'Error: Lib {0} and/or it''s header was not found in the target library paths.'.format(lib)
			Exit(1)
	
	conf.Finish()


env.Program(Split('IRCbot.c users.c'))
env.Program('ghdecode.c')
