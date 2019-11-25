project('bearwasm', 'cpp', default_options: ['cpp_std=c++17'])

cpp_sources = files('src/Interpreter.cpp',
		'src/main.cpp',
		'src/Module.cpp',
		'src/Util.cpp',
		'src/VirtualMachine.cpp')
cpp_includes = include_directories('include')

nasm_source = files('src/ASMInterpreter.asm')
nasm = find_program('nasm')
nasm_gen = generator(nasm, output: '@BASENAME@.o', arguments: ['-f', 'macho64', '-g', '-F', 'dwarf', '@INPUT@', '-o', '@OUTPUT@'])
nasm_objects = nasm_gen.process(nasm_source)

executable('bearwasm', [cpp_sources, nasm_objects], include_directories: cpp_includes)
