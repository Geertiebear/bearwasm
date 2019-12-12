project('bearwasm', 'cpp', default_options: ['cpp_std=c++17'])

cpp_sources = files('src/Interpreter.cpp',
		'src/main.cpp',
		'src/Module.cpp',
		'src/Util.cpp',
		'src/VirtualMachine.cpp')
cpp_includes = include_directories('include')

executable('bearwasm', [cpp_sources], include_directories: cpp_includes)
