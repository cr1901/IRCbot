env = Environment()
env['CFLAGS'] = '-ansi -Wall -Wextra'

if ARGUMENTS.get('DEBUG', 0):
	env.Append(CFLAGS = ' -pedantic -O0 -gdwarf-2')
	env.Append(LINKFLAGS = ' -gdwarf-2') 
else:
	env.Append(CFLAGS = ' -pedantic-errors -O2')
	env.Append(CPPDEFINES = ['NDEBUG', 'NDEBUG_MESSAGES']) 

env.Program(Split('IRCbot.c users.c'))
