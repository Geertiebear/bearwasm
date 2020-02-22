project('bearwasm', 'cpp', default_options: ['cpp_std=c++17'])

bearwasm_sources = files('src/Interpreter.cpp',
		'src/Module.cpp',
		'src/Util.cpp',
		'src/VirtualMachine.cpp',
		'src/libc.cpp')
linux_sources = files('src/main.cpp')
cpp_includes = include_directories('include')

frigg = subproject('frigg', default_options: ['frigg_no_install=true'])
frigg_dep = frigg.get_variable('frigg_dep')
cxxshim = subproject('cxxshim')
cxxshim_dep = cxxshim.get_variable('cxxshim_dep')

bearwasm_lib = static_library('bearwasm', [bearwasm_sources],
  include_directories: cpp_includes, dependencies: [frigg_dep, cxxshim_dep],
  cpp_args: ['-ffreestanding', '-fno-exceptions', '-fno-rtti'])

executable('bearwasm', [linux_sources], include_directories: cpp_includes,
  link_with: bearwasm_lib, dependencies: [frigg_dep])
