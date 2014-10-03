import os

dummy_env = DefaultEnvironment()

vars = Variables(['variables.cache', 'settings.py'])
vars.Add(BoolVariable('DEBUG', 'Set to 1 to set Debug options.', 0))
vars.Add(BoolVariable('OS_ENV', 'Allow SCons to use the System Path and OS Environment (useful for distcc)', 0))
vars.Add('CC', 'Choose your C Compiler. Defaults to DefaultEnvironment()[''CC''].', dummy_env['CC'])
vars.Add(PathVariable('INCLUDE_PATH', 'Specify extra paths headers', None))
vars.Add(PathVariable('LIBRARY_PATH', 'Specify extra paths for linking', None))
vars.Add(PathVariable('RUNTIME_PATH', 'Specify extra paths for runtime loading (rpath- useful for pkgsrc)', None))
#vars.Add('CXX', 'Choose your C++ Compiler. Defaults to DefaultEnvironment()[''CXX''].', dummy_env['CXX'])

env = Environment(variables = vars)

vars.Save('variables.cache', env)
Help(vars.GenerateHelpText(env))

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
	env.Append(CFLAGS = ' -pedantic -O0 -gdwarf-2 -Wno-unused-parameter'\
		' -Wno-unused-but-set-parameter')
	env.Append(LINKFLAGS = ' -gdwarf-2') 
else:
	env.Append(CFLAGS = ' -pedantic -O2')
	env.Append(CPPDEFINES = ['NDEBUG', 'NDEBUG_MESSAGES', 'NPRINT_OUTPUT']) 
	

#conf = Configure(env, config_h='cc_config.h', clean=False, help=False) #Appears to be broken...
if not env.GetOption('clean') and not env.GetOption('help'):
	conf = Configure(env, config_h='cc_config.h', custom_tests = \
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
		
	for db_lib, db_header in zip(['db', 'db2', 'db3', 'db4', 'db5'], \
		['db.h', 'db2/db.h', 'db3/db.h', 'db4/db.h', 'db5/db.h']):
		if conf.CheckLibWithHeader(db_lib, db_header, 'C', autoadd=True):
			break
	else:
		print 'Error: Berkeley DB and/or its headers were not found in the target library paths.'
		Exit(1)
	
	if not conf.CheckLibWithHeader('jansson', 'jansson.h', 'C', autoadd=True):
		print 'Error: Jansson and/or its headers were not found in the target library paths.'
		Exit(1)
	
	conf.Finish()

#Repository('EPROMs') 
#EPROMs = Glob('*#*')


IRCbot = env.Program(Split('IRCbot.c debug.c sockwrap.c tokparse.c users.c'))
ghdecode = env.Program('ghdecode.c')
Default([IRCbot, ghdecode])

#This is somewhat of a hack... it's better to use an Emitter...
#Ask on mailing list why Repository sources default to NOT building in 
#repo dir (or if this is specific to custom builders).
#gh_questions = Builder(action = './ghdecode $SOURCE.rsrcdir/$TARGET $SOURCE', suffix='.json', single_source = True)
#env.Append(BUILDERS = {'GHJSON' : gh_questions})
#questions = env.GHJSON(EPROMs)
#Alias('questions', questions)

