#!/usr/bin/python
# -*- coding: utf-8; -*-

import xml.dom.minidom
from xml.dom.minidom import Node
import os

class Callable:
    def __init__(self, func):
        self.__call__ = func

def main():
	print 'reading with "doxygen xml.doxygen" generated files'

	libs3d = xml.dom.minidom.parse("xml/s3d_8h.xml")
	libs3dw = xml.dom.minidom.parse("xml/s3dw_8h.xml")

	libs3d_func = extract_functions(libs3d)
	libs3dw_func = extract_functions(libs3dw)

	libs3d_struct = extract_structs(libs3d)
	libs3dw_struct = extract_structs(libs3dw)

	libs3d_typedef = extract_typedefs(libs3d)
	libs3dw_typedef = extract_typedefs(libs3dw)

	docbook_functions.generate('libs3d', "s3d.h", libs3d_func)
	docbook_functions.generate('libs3dw', "s3dw.h", libs3dw_func)

	docbook_structs.generate('libs3d', libs3d_struct)
	docbook_structs.generate('libs3dw', libs3dw_struct)

	docbook_typedefs.generate('libs3d', libs3d_typedef)
	docbook_typedefs.generate('libs3dw', libs3dw_typedef)

	rm_files('./manpages/man3/')
	manpage_functions.generate("s3d.h", libs3d_func)
	manpage_functions.generate("s3dw.h", libs3dw_func)

	rm_files('./manpages/man9/')
	manpage_structs.generate("s3d.h", libs3d_struct)
	manpage_structs.generate("s3dw.h", libs3dw_struct)


"""
Remove all files in a directory and creates it when it does not exist
"""
def rm_files(top):
	try:
		os.makedirs(top)
	except OSError:
		pass
	for entry in os.listdir(top):
		if os.path.isfile(os.path.join(top, entry)):
			os.remove(os.path.join(top, entry))

"""
Removes _ from beginning of a string
"""
def cleanup_stringbegin(string):
	new_str = string
	while new_str[0] in ['_']:
		new_str = new_str[1:]
	return new_str
	

"""
Generate text from all childNodes
"""
def get_text(node):
	t = ''
	for node in node.childNodes:
		if node.nodeType == Node.TEXT_NODE:
			t += node.data
		else:
			t += get_text(node)
	return t


"""
Generate linear list of text and section types
"""
def get_text_complex(node):
	t = []
	for node in node.childNodes:
		if node.nodeType == Node.TEXT_NODE:
			t.append(node.data)
		else:
			if node.nodeName == 'sp':
				t.append(" ")
			elif node.nodeName == 'para':
				t.append({'type': 'para', 'text': ''})
				t += get_text_complex(node)
			elif node.nodeName == 'programlisting':
				t.append({'type': 'programlisting', 'text': ''})
				t += get_text_complex(node)
				t.append({'type': 'para', 'text': ''})
			elif node.nodeName == 'simplesect':
				if node.attributes['kind'].nodeValue == 'remark':
					t.append({'type': 'warning', 'text': ''})
					t += get_text_complex(node)
					t.append({'type': 'para', 'text': ''})
				else:
					t.append({'type': 'para', 'text': ''})
					t += get_text_complex(node)
			else:
				t += get_text_complex(node)
	return t

"""
Convert linear list of text and section types to list of section types with corresponding text
"""
def complex2simplearray(objects):
	cur_object = 0
	array = []
	for element in objects:
		if type(element) != dict:
			# add text to last section type
			if cur_object == 0:
				array.append({'type': 'para', 'text': element})
				cur_object = array[0]
			else:
				cur_object['text'] += element
		else:
			# add new section type
			if element['type'] == 'para' and len(array) != 0 and array[-1]['type'] in ['warning']:
				# ignore para inside warning and add text to last section type
				cur_object['text'] += element['text']
			else:
				cur_object = element
				array.append(element)

	return array

def remove_exportdefinitions(function_return):
	exports = ["S3DEXPORT", "S3DWEXPORT"]
	for export in exports:
		if function_return[:len(export)] == export:
			return function_return[len(export):].strip()

"""
Create new node with tag name node_type and add it to father
"""
def create_append(document, father, node_type):
	t = document.createElement(node_type)
	father.appendChild(t)
	return t

"""
Create new text node with text and add it to father
"""
def create_append_text(document, father, text):
	t = document.createTextNode(text)
	father.appendChild(t)
	return t

"""
Append complex help section to sect
"""
def help_append(sgml, sect, help):
	for p in help:
		if p['text'] != '':
			if p['type'] in ['warning']:
				# add para in warning before adding help text
				extra_para = create_append(sgml, sect, p['type'])
				para = create_append(sgml, extra_para, 'para')
				create_append_text(sgml, para, p['text'])
			else:
				if p['text'].strip() == '':
					continue
				para = create_append(sgml, sect, p['type'])
				create_append_text(sgml, para, p['text'])

"""
Extract function informations from doxygen dom
"""
def extract_functions(dom):
	functionlist = []
	for node in dom.getElementsByTagName("memberdef"):
		# find nodes with functions information
		if node.attributes['kind'].nodeValue != 'function':
			continue

		function = {'return': '', 'name': '', 'param': [], 'help': []}
		for node2 in node.childNodes:
			if node2.nodeName == "name":
				function['name'] = get_text(node2)

			if node2.nodeName == "type":
				function['return'] = remove_exportdefinitions(get_text(node2))

			if node2.nodeName == "param":
				param = {'type' : '', 'declname' : '', 'array' : ''}
				for n in node2.childNodes:
					if n.nodeName == 'type':
						param['type'] = get_text(n)

					if n.nodeName == 'declname':
						param['declname'] = get_text(n)

					if n.nodeName == 'array':
						param['array'] = get_text(n)

				function['param'].append(param)

			if node2.nodeName == 'detaileddescription':
				help = get_text_complex(node2)
				function['help'] = complex2simplearray(help)

		functionlist.append(function)

	return functionlist

"""
Extract struct informations from doxygen dom
"""
def extract_structs(dom):
	structlist = []
	# find refs (names of xml files) of structs
	for node in dom.getElementsByTagName("innerclass"):
		struct = {'name': '', 'ref': '', 'elements': [], 'help': []}
		struct['name'] = get_text(node)
		struct['ref'] = node.attributes['refid'].nodeValue
		structlist.append(struct)

	# open xml files and extract informations from them
	for struct in structlist:
		dom = xml.dom.minidom.parse("xml/"+struct['ref']+".xml")

		for node in dom.getElementsByTagName('compounddef')[0].childNodes:
			if node.nodeName == 'detaileddescription':
				help = get_text_complex(node)
				struct['help'] = complex2simplearray(help)

		for node in dom.getElementsByTagName("memberdef"):
			element = {'type': '', 'name' : '', 'help': []}
			for node2 in node.childNodes:
				if node2.nodeName == "name":
					element['name'] = get_text(node2)

				if node2.nodeName == "type":
					element['type'] = get_text(node2)

				if node2.nodeName == 'detaileddescription':
					help = get_text_complex(node2)
					element['help'] = complex2simplearray(help)

			struct['elements'].append(element)

	return structlist

"""
Extract typedef informations from doxygen dom
"""
def extract_typedefs(dom):
	typedeflist = []
	for node in dom.getElementsByTagName("memberdef"):
		# find nodes with typedef information
		if node.attributes['kind'].nodeValue != 'typedef':
			continue

		typedef = {'name': '', 'definition': '', 'help': []}
		for node2 in node.childNodes:
			if node2.nodeName == 'name':
				typedef['name'] = get_text(node2)

			if node2.nodeName == 'definition':
				typedef['definition'] = get_text(node2)

			if node2.nodeName == 'detaileddescription':
				help = get_text_complex(node2)
				typedef['help'] = complex2simplearray(help)

		typedeflist.append(typedef)

	return typedeflist

class docbook_functions:
	"""
	Generate docbook file with informations to all functions
	"""
	def generate(name, synopsis, functionlist):
		func_file = open(name+'/functions.docbook', "w")
		for func in functionlist:
			sgml = docbook_functions.generate_sgml(func, synopsis)
			sgml.writexml(func_file)
		func_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific function
	"""
	def generate_sgml(function, synopsis):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, sgml, 'sect2')
		sect2.setAttribute('id', function['name'])

		title = create_append(sgml, sect2, 'title')
		create_append_text(sgml, title, function['name'])

		# synopsis
		funcsynopsis = create_append(sgml, sect2, 'funcsynopsis')
		funcsynopsisinfo = create_append(sgml, funcsynopsis, 'funcsynopsisinfo')
		create_append_text(sgml, funcsynopsisinfo, "#include <"+synopsis+">")

		# prototype
		funcprototype = create_append(sgml, funcsynopsis, 'funcprototype')

		funcdef = create_append(sgml, funcprototype, 'funcdef')
		create_append_text(sgml, funcdef, function['return']+" ")

		func = create_append(sgml, funcdef, 'function')
		create_append_text(sgml, func, function['name'])

		# add parameter to function definition
		param_num = len(function['param'])
		for i in range(0, param_num):
			paramdef = create_append(sgml, funcprototype, 'paramdef')

			create_append_text(sgml, paramdef, function['param'][i]['type'])

			if function['param'][i]['declname'] != '':
				if function['param'][i]['type'][-1:] != "*":
					# dont add space between * and name
					create_append_text(sgml, paramdef, " ")
				parameter = create_append(sgml, paramdef, 'parameter')
				create_append_text(sgml, parameter, function['param'][i]['declname'])

			if function['param'][i]['array'] != '':
				create_append_text(sgml, paramdef, function['param'][i]['array'])

		# add help to function
		help_append(sgml, sect2, function['help'])

		return sect2

	# make functions "static"
	generate = Callable(generate)
	generate_sgml = Callable(generate_sgml)

class docbook_structs:
	"""
	Generate docbook file with informations to all structs
	"""
	def generate(name, structlist):
		struct_file = open(name+'/structs.docbook', "w")
		for struct in structlist:
			sgml = docbook_structs.generate_sgml(struct)
			sgml.writexml(struct_file)
		struct_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific struct
	"""
	def generate_sgml(struct):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, sgml, 'sect2')
		sect2.setAttribute('id', 'struct'+struct['name'])

		title = create_append(sgml, sect2, 'title')
		create_append_text(sgml, title, 'struct '+struct['name'])

		# add definition of struct
		programlisting = create_append(sgml, sect2, 'programlisting')
		create_append_text(sgml, programlisting, 'struct '+struct['name']+' {\n')
		for element in struct['elements']:
			create_append_text(sgml, programlisting, '\t'+element['type'])
			if element['type'][-1:] != "*":
				# dont add space between * and name
				create_append_text(sgml, programlisting, " ")
			create_append_text(sgml, programlisting, element['name']+';\n')
		create_append_text(sgml, programlisting, '}')

		# add help to struct
		help_append(sgml, sect2, struct['help'])

		# add list of struct members with their help
		variablelist = create_append(sgml, sect2, 'variablelist')
		for element in struct['elements']:
			# ignore members with empty help texts
			if len(element['help']) == 1 and element['help'][0]['text'].strip() == '':
				continue

			varlistentry = create_append(sgml, variablelist, 'varlistentry')
			term = create_append(sgml, varlistentry, 'term')
			create_append_text(sgml, term, element['name'])
			listitem = create_append(sgml, varlistentry, 'listitem')

			# add help to struct member
			help_append(sgml, listitem, element['help'])

		# remove empty variablelist
		if len(variablelist.childNodes) == 0:
			sect2.removeChild(variablelist)

		return sect2

	# make functions "static"
	generate = Callable(generate)
	generate_sgml = Callable(generate_sgml)

class docbook_typedefs:
	"""
	Generate docbook file with informations to all typedefs
	"""
	def generate(name, typedeflist):
		typedef_file = open(name+'/typedefs.docbook', "w")
		for typedef in typedeflist:
			sgml = docbook_typedefs.generate_sgml(typedef)
			sgml.writexml(typedef_file)
		typedef_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific typedef
	"""
	def generate_sgml(typedef):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, sgml, 'sect2')
		sect2.setAttribute('id', typedef['name'])

		title = create_append(sgml, sect2, 'title')
		create_append_text(sgml, title, 'typedef '+typedef['name'])

		# add definition of typedef
		programlisting = create_append(sgml, sect2, 'programlisting')
		create_append_text(sgml, programlisting, typedef['definition'])

		# add help to typedef
		help_append(sgml, sect2, typedef['help'])

		return sect2

	# make functions "static"
	generate = Callable(generate)
	generate_sgml = Callable(generate_sgml)

class manpage_functions:
	"""
	Generate manpage docbook file with informations to functions
	"""
	def generate(synopsis, functionlist):
		for func in functionlist:
			func_file = open('./manpages/man3/'+cleanup_stringbegin(func['name'])+'.sgml', "w")
			func_file.write('<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook V4.1//EN">\n')
			sgml = manpage_functions.generate_sgml(func, synopsis)
			sgml.writexml(func_file)
			func_file.close()

	"""
	Generate manpage docbook dom with informations to a specific function
	"""
	def generate_sgml(function, synopsis):
		sgml = xml.dom.minidom.Document()

		refentry = create_append(sgml, sgml, 'refentry')
		refentry.setAttribute('id', cleanup_stringbegin(function['name']))

		refmeta = create_append(sgml, refentry, 'refmeta')

		refentrytitle = create_append(sgml, refmeta, 'refentrytitle')
		create_append_text(sgml, refentrytitle, function['name'])

		manvolnum = create_append(sgml, refmeta, 'manvolnum')
		create_append_text(sgml, manvolnum, '3')

		refnamediv = create_append(sgml, refentry, 'refnamediv')

		refname = create_append(sgml, refnamediv, 'refname')
		create_append_text(sgml, refname, function['name'])
		refpurpose = create_append(sgml, refnamediv, 'refpurpose')
		create_append_text(sgml, refpurpose, "")

		# synopsis
		refsynopsisdiv = create_append(sgml, refentry, 'refsynopsisdiv')
		funcsynopsis = create_append(sgml, refsynopsisdiv, 'funcsynopsis')
		funcsynopsisinfo = create_append(sgml, funcsynopsis, 'funcsynopsisinfo')
		create_append_text(sgml, funcsynopsisinfo, "#include <"+synopsis+">")

		# prototype
		funcprototype = create_append(sgml, funcsynopsis, 'funcprototype')

		funcdef = create_append(sgml, funcprototype, 'funcdef')
		create_append_text(sgml, funcdef, function['return']+" ")

		func = create_append(sgml, funcdef, 'function')
		create_append_text(sgml, func, function['name'])

		# add parameter to function definition
		
		param_num = len(function['param'])
		for i in range(0, param_num):
			paramdef = create_append(sgml, funcprototype, 'paramdef')
			create_append_text(sgml, paramdef, "\t"+function['param'][i]['type'])

			if function['param'][i]['declname'] != '':
				if function['param'][i]['type'][-1:] != "*":
					# dont add space between * and name
					create_append_text(sgml, paramdef, " ")
				parameter = create_append(sgml, paramdef, 'parameter')
				create_append_text(sgml, parameter, function['param'][i]['declname'])

			if function['param'][i]['array'] != '':
				create_append_text(sgml, paramdef, function['param'][i]['array'])

		# add help to function
		refsect1 = create_append(sgml, refentry, 'refsect1')
		title = create_append(sgml, refsect1, 'title')
		create_append_text(sgml, title, "Description")
		help_append(sgml, refsect1, function['help'])

		return refentry

	# make functions "static"
	generate = Callable(generate)
	generate_sgml = Callable(generate_sgml)

class manpage_structs:
	"""
	Generate manpage docbook file with informations to all structs
	"""
	def generate(synopsis, structlist):
		for func in structlist:
			func_file = open('./manpages/man9/'+cleanup_stringbegin(func['name'])+'.sgml', "w")
			func_file.write('<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook V4.1//EN">\n')
			sgml = manpage_structs.generate_sgml(func, synopsis)
			sgml.writexml(func_file)
			func_file.close()

	"""
	Generate manpage docbook dom with informations to a specific struct
	"""
	def generate_sgml(struct, synopsis):
		sgml = xml.dom.minidom.Document()

		refentry = create_append(sgml, sgml, 'refentry')
		refentry.setAttribute('id', cleanup_stringbegin(struct['name']))

		refmeta = create_append(sgml, refentry, 'refmeta')

		refentrytitle = create_append(sgml, refmeta, 'refentrytitle')
		create_append_text(sgml, refentrytitle, struct['name'])

		manvolnum = create_append(sgml, refmeta, 'manvolnum')
		create_append_text(sgml, manvolnum, '9')

		refnamediv = create_append(sgml, refentry, 'refnamediv')

		refname = create_append(sgml, refnamediv, 'refname')
		create_append_text(sgml, refname, 'struct ' + struct['name'])
		refpurpose = create_append(sgml, refnamediv, 'refpurpose')
		create_append_text(sgml, refpurpose, "")

		# synopsis
		refsynopsisdiv = create_append(sgml, refentry, 'refsynopsisdiv')
		funcsynopsis = create_append(sgml, refsynopsisdiv, 'funcsynopsis')
		funcsynopsisinfo = create_append(sgml, funcsynopsis, 'funcsynopsisinfo')
		create_append_text(sgml, funcsynopsisinfo, "#include <"+synopsis+">")

		# add definition of struct
		refsect1 = create_append(sgml, refentry, 'refsect1')
		title = create_append(sgml, refsect1, 'title')
		create_append_text(sgml, title, "Structure Members")
		
		programlisting = create_append(sgml, refsect1, 'programlisting')
		create_append_text(sgml, programlisting, 'struct '+struct['name']+' {\n')
		for element in struct['elements']:
			create_append_text(sgml, programlisting, '\t'+element['type'])
			if element['type'][-1:] != "*":
				# dont add space between * and name
				create_append_text(sgml, programlisting, " ")
			create_append_text(sgml, programlisting, element['name']+';\n')
		create_append_text(sgml, programlisting, '}')

		# add help to struct
		refsect1 = create_append(sgml, refentry, 'refsect1')
		title = create_append(sgml, refsect1, 'title')
		create_append_text(sgml, title, "Description")
		help_append(sgml, refsect1, struct['help'])

		# add list of struct members with their help
		variablelist = create_append(sgml, refsect1, 'variablelist')
		for element in struct['elements']:
			# ignore members with empty help texts
			if len(element['help']) == 1 and element['help'][0]['text'].strip() == '':
				continue

			varlistentry = create_append(sgml, variablelist, 'varlistentry')
			term = create_append(sgml, varlistentry, 'term')
			create_append_text(sgml, term, element['name'])
			listitem = create_append(sgml, varlistentry, 'listitem')

			# add help to struct member
			help_append(sgml, listitem, element['help'])

		# remove empty variablelist
		if len(variablelist.childNodes) == 0:
			refsect1.removeChild(variablelist)

		return refentry

	# make functions "static"
	generate = Callable(generate)
	generate_sgml = Callable(generate_sgml)

if __name__ == '__main__':
	main()
