env = Environment()
env['CFLAGS'] = '-pedantic -Wall -Wextra'

if ARGUMENTS.get('DEBUG', 0):
	env.Append(CFLAGS = ' -O0 -gdwarf-2')
	env.Append(LINKFLAGS = ' -gdwarf-2') 
else:
	env.Append(CFLAGS = ' -O2')

env.Program('IRCbot.c')
