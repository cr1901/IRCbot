import os

dummy_env = DefaultEnvironment()

vars = Variables()
vars.Add(BoolVariable('DEBUG', 'Set to 1 to set Debug options.', 0))
vars.Add(BoolVariable('SYSTEM_PATH', 'Allow SCons to use the System Path and OS Environment (useful for distcc)', 0))
vars.Add('CC', 'Choose your C Compiler. Defaults to DefaultEnvironment()[''CC''].', dummy_env['CC'])
#vars.Add('CXX', 'Choose your C++ Compiler. Defaults to DefaultEnvironment()[''CXX''].', dummy_env['CXX'])

"""
DISTCC
real    0m7.926s
user    0m5.350s
sys     0m0.540s

WITHOUT_DISTCC
real    0m9.474s
user    0m7.850s
sys     0m0.650s
"""

env = Environment(variables = vars) 
Help(vars.GenerateHelpText(env))

if env['SYSTEM_PATH']:
	env['ENV'] = os.environ

#env['CFLAGS'] = '-ansi -Wall -Wextra'
env['CFLAGS'] = '-Wall -Wextra'

if ARGUMENTS.get('DEBUG', False):
	env.Append(CFLAGS = ' -pedantic -O0 -gdwarf-2')
	env.Append(LINKFLAGS = ' -gdwarf-2') 
else:
	env.Append(CFLAGS = ' -pedantic-errors -O2')
	env.Append(CPPDEFINES = ['NDEBUG', 'NDEBUG_MESSAGES']) 
	
#conf = Configure(env, config_h='cc_config.h', clean=False, help=False)
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
	
	for lib_header, lib in zip(['db.h'], ['db']):
		if not conf.CheckLibWithHeader(lib, lib_header, 'C', autoadd=True):
			print 'Error: Lib {0} and/or it''s header was not found in the target library paths.'.format(lib)
			Exit(1)
	
	conf.Finish()


env.Program(Split('IRCbot.c users.c'))
