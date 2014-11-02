import os

dummy_env = DefaultEnvironment()

vars = Variables(['variables.cache', 'settings.py'])
vars.Add(BoolVariable('DEBUG', 'Set to 1 to set Debug options.', 0))
vars.Add(BoolVariable('OS_ENV', 'Allow SCons to use the System Path and OS Environment (useful for distcc)', 0))
vars.Add('CC', 'Choose your C Compiler. Defaults to DefaultEnvironment()[''CC''].', dummy_env['CC'])
vars.Add(PathVariable('INCLUDE_PATH', 'Specify extra paths headers', None, PathVariable.PathAccept))
vars.Add(PathVariable('LIBRARY_PATH', 'Specify extra paths for linking', None, PathVariable.PathAccept))
vars.Add(PathVariable('RUNTIME_PATH', 'Specify extra paths for runtime loading (rpath- useful for pkgsrc)', None, PathVariable.PathAccept))
#vars.Add('CXX', 'Choose your C++ Compiler. Defaults to DefaultEnvironment()[''CXX''].', dummy_env['CXX'])

env = Environment(variables = vars)

vars.Save('variables.cache', env)
Help(vars.GenerateHelpText(env))

SetBuildEnvVars(env)
RunConfigureTests(env)



#Repository('EPROMs') 
#EPROMs = Glob('*#*')
SConscript('src/SConscript', exports = ['env'])



#This is somewhat of a hack... it's better to use an Emitter...
#Ask on mailing list why Repository sources default to NOT building in 
#repo dir (or if this is specific to custom builders).
#gh_questions = Builder(action = './ghdecode $SOURCE.rsrcdir/$TARGET $SOURCE', suffix='.json', single_source = True)
#env.Append(BUILDERS = {'GHJSON' : gh_questions})
#questions = env.GHJSON(EPROMs)
#Alias('questions', questions)

