#!/usr/bin/python
# -*- coding: utf-8; -*-

import xml.dom.minidom
from xml.dom.minidom import Node
import os
import re

def main():
	print('reading with "doxygen xml.doxygen" generated files')
	refidlist = []

	libs3d = xml.dom.minidom.parse("xml/s3d_8h.xml")
	libs3dw = xml.dom.minidom.parse("xml/s3dw_8h.xml")

	libs3d_func = extract_functions(libs3d)
	libs3dw_func = extract_functions(libs3dw)
	refidlist += libs3d_func + libs3dw_func

	libs3d_struct = extract_structs(libs3d)
	libs3dw_struct = extract_structs(libs3dw)
	refidlist += libs3d_struct + libs3dw_struct

	libs3d_typedef = extract_typedefs(libs3d)
	libs3dw_typedef = extract_typedefs(libs3dw)
	refidlist += libs3d_typedef + libs3dw_typedef

	docbook_functions.generate('libs3d', "s3d.h", libs3d_func, refidlist)
	docbook_functions.generate('libs3dw', "s3dw.h", libs3dw_func, refidlist)

	docbook_structs.generate('libs3d', libs3d_struct, refidlist)
	docbook_structs.generate('libs3dw', libs3dw_struct, refidlist)

	docbook_typedefs.generate('libs3d', libs3d_typedef, refidlist)
	docbook_typedefs.generate('libs3dw', libs3dw_typedef, refidlist)

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
Add references to docbook dom
"""
def link_refids(dom, refidlist):
	for refitem in refidlist:
		link_refid(dom, refitem['name'], refitem['id'])

"""
Search text in dom for name to replace it by link to refid
"""
def link_refid(dom, name, refid):
	num_nodes = len(dom.childNodes)
	i = 0
	while i < num_nodes:
		node = dom.childNodes[i]
		if node.nodeType == Node.TEXT_NODE:
			string = node.data
			valid_suround = ['', ',', '.', ' ', '(', ')', '\n', '\r', '!', '?']
			found = 0
			while found != -1:
				found =  string.find(name, found)
				if found != -1:
					next_char = ''
					prev_char = ''

					# check for valid surounding chars
					if (found + len(name)) < len(string):
						next_char = string[found + len(name)]
					if found > 0:
						prev_char = string[found - 1]

					if next_char in valid_suround and prev_char in valid_suround:
						# suroundings chars ok -> safe beginning and link
						create_before_text(dom, string[:found], node)
						link = create_before(dom, 'link', node)
						link.setAttribute('linkend', refid)
						create_append_text(link, name)
						num_nodes = num_nodes + 2
						i = i + 2

						# continue search after laster found
						string = string[(found + len(name)):]
						found = 0
					else:
						#no valid surounding chars found -> move on
						found = found + 1

				else:
					# finishes search and can now remove old strings
					create_before_text(dom, string, node)
					dom.removeChild(node)
		else:
			if node.nodeName not in ['funcprototype', 'title']:
				link_refid(node, name, refid)
		i = i + 1

class detaileddescription:
	t = []

	def __init__(self, node):
		self.t = []
		self.__get_text_complex(node)
		self.__complex2simplearray()

	"""
	Generate linear list of text and section types
	"""
	def __get_text_complex(self, node):
		for node in node.childNodes:
			if node.nodeType == Node.TEXT_NODE:
				self.t.append(node.data)
			else:
				if node.nodeName == 'sp':
					self.t.append(" ")
				elif node.nodeName == 'para':
					self.t.append({'type': 'para', 'text': ''})
					self.__get_text_complex(node)
				elif node.nodeName == 'programlisting':
					self.t.append({'type': 'programlisting', 'text': ''})
					self.__get_text_complex(node)
					self.t.append({'type': 'para', 'text': ''})
				elif node.nodeName == 'simplesect':
					if node.attributes['kind'].nodeValue == 'remark':
						self.t.append({'type': 'warning', 'text': ''})
						self.__get_text_complex(node)
						self.t.append({'type': 'para', 'text': ''})
					else:
						self.t.append({'type': 'para', 'text': ''})
						self.__get_text_complex(node)
				else:
					self.__get_text_complex(node)

	"""
	Convert linear list of text and section types to list of section types with corresponding text
	"""
	def __complex2simplearray(self):
		cur_object = 0
		array = []
		for element in self.t:
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

		self.t = array

	"""
	Append complex help section to dom
	"""
	def dom_append(self, sect):
		for p in self.t:
			if p['text'] != '':
				if p['type'] in ['warning']:
					# add para in warning before adding help text
					extra_para = create_append(sect, p['type'])
					para = create_append(extra_para, 'para')
					create_append_text(para, p['text'])
				else:
					if p['text'].strip() == '':
						continue
					para = create_append(sect, p['type'])
					create_append_text(para, p['text'])

	def isempty(self):
		return (len(self.t) == 0) or (len(self.t) == 1 and self.t[0]['text'].strip() == '')

class function_param:
	def __init__(self, node):
		self.param = {'type' : '', 'declname' : '', 'array' : ''}
		for n in node.childNodes:
			if n.nodeName == 'type':
				self.param['type'] = get_text(n)

			if n.nodeName == 'declname':
				self.param['declname'] = get_text(n)

			if n.nodeName == 'array':
				self.param['array'] = get_text(n)

	def dom_append(self, funcprototype, intent = ""):
		paramdef = create_append(funcprototype, 'paramdef')

		create_append_text(paramdef, intent+self.param['type'])

		if self.param['declname'] != '':
			if self.param['type'][-1:] != "*":
				# dont add space between * and name
				create_append_text(paramdef, " ")
			parameter = create_append(paramdef, 'parameter')
			create_append_text(parameter, self.param['declname'])

		if self.param['array'] != '':
			create_append_text(paramdef, self.param['array'])

	def is_void(self):
		if self.param['type'] == 'void' and self.param['declname'] == '':
			return 1
		else:
			return 0

class struct_element:
	def __init__(self, node):
		self.element = {'type': '', 'name' : '', 'help': []}
		for node2 in node.childNodes:
			if node2.nodeName == "name":
				self.element['name'] = get_text(node2)

			if node2.nodeName == "type":
				self.element['type'] = get_text(node2)

			if node2.nodeName == 'detaileddescription':
				self.element['help'] = detaileddescription(node2)

	def dom_append(self, programlisting):
		create_append_text(programlisting, '\t'+self.element['type'])
		if self.element['type'][-1:] != "*":
			# dont add space between * and name
			create_append_text(programlisting, " ")
		create_append_text(programlisting, self.element['name']+';\n')

	def dom_append_help(self, variablelist):
		# ignore members with empty help texts
		if self.element['help'].isempty():
			return

		varlistentry = create_append(variablelist, 'varlistentry')
		term = create_append(varlistentry, 'term')
		create_append_text(term, self.element['name'])
		listitem = create_append(varlistentry, 'listitem')

		# add help to struct member
		self.element['help'].dom_append(listitem)

def remove_exportdefinitions(function_return):
	exports = ["S3DEXPORT", "S3DWEXPORT"]
	for export in exports:
		if function_return[:len(export)] == export:
			return function_return[len(export):].strip()

"""
Create new node with tag name node_type and add it to father
"""
def create_append(father, node_type):
	if father.ownerDocument:
		t = father.ownerDocument.createElement(node_type)
	else:
		# no father -> so it must be a document
		t = father.createElement(node_type)

	father.appendChild(t)
	return t

"""
Create new processing instruction with tag name node_type and add it to father
"""
def create_append_pi(father, node_type, instruction):
	if father.ownerDocument:
		t = father.ownerDocument.createProcessingInstruction(node_type, instruction)
	else:
		# no father -> so it must be a document
		t = father.createProcessingInstruction(node_type, instruction)

	father.appendChild(t)
	return t

"""
Create new text node with text and add it to father
"""
def create_append_text(father, text):
	if father.ownerDocument:
		t = father.ownerDocument.createTextNode(text)
	else:
		# no father -> so it must be a document
		t = father.createTextNode(text)
		
	father.appendChild(t)
	return t

"""
Create new node with tag name node_type and add it to father before refnode
"""
def create_before(father, node_type, refnode):
	if father.ownerDocument:
		t = father.ownerDocument.createElement(node_type)
	else:
		# no father -> so it must be a document
		t = father.createElement(text)

	father.insertBefore(t, refnode)
	return t

"""
Create new text node with text and add it to father before refnode
"""
def create_before_text(father, text, refnode):
	if father.ownerDocument:
		t = father.ownerDocument.createTextNode(text)
	else:
		# no father -> so it must be a document
		t = father.createTextNode(text)

	father.insertBefore(t, refnode)
	return t

"""
Extract function informations from doxygen dom
"""
def extract_functions(dom):
	functionlist = []
	for node in dom.getElementsByTagName("memberdef"):
		# find nodes with functions information
		if node.attributes['kind'].nodeValue != 'function':
			continue

		function = {'return': '', 'name': '', 'id': '', 'param': [], 'brief': '', 'help': []}
		for node2 in node.childNodes:
			if node2.nodeName == "name":
				function['name'] = get_text(node2)
				function['id'] = function['name']

			if node2.nodeName == "type":
				function['return'] = remove_exportdefinitions(get_text(node2))

			if node2.nodeName == "param":
				function['param'].append(function_param(node2))

			if node2.nodeName == "briefdescription":
				function['brief'] = get_text(node2)

			if node2.nodeName == 'detaileddescription':
				function['help'] = detaileddescription(node2)

		functionlist.append(function)

	return functionlist

"""
Extract struct informations from doxygen dom
"""
def extract_structs(dom):
	structlist = []
	# find refs (names of xml files) of structs
	for node in dom.getElementsByTagName("innerclass"):
		struct = {'name': '', 'id': '', 'ref': '', 'elements': [], 'brief': '', 'help': []}
		struct['name'] = get_text(node)
		struct['id'] = 'struct'+struct['name']
		struct['ref'] = node.attributes['refid'].nodeValue
		structlist.append(struct)

	# open xml files and extract informations from them
	for struct in structlist:
		dom = xml.dom.minidom.parse("xml/"+struct['ref']+".xml")

		for node in dom.getElementsByTagName('compounddef')[0].childNodes:
			if node.nodeName == "briefdescription":
				struct['brief'] = get_text(node)

			if node.nodeName == 'detaileddescription':
				struct['help'] = detaileddescription(node)

		for node in dom.getElementsByTagName("memberdef"):
			struct['elements'].append(struct_element(node))

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

		typedef = {'name': '', 'id': '', 'definition': '', 'help': []}
		for node2 in node.childNodes:
			if node2.nodeName == 'name':
				typedef['name'] = get_text(node2)
				typedef['id'] = typedef['name']

			if node2.nodeName == 'definition':
				typedef['definition'] = get_text(node2)

			if node2.nodeName == 'detaileddescription':
				typedef['help'] = detaileddescription(node2)

		typedeflist.append(typedef)

	return typedeflist

class docbook_functions:
	"""
	Generate docbook file with informations to all functions
	"""
	def generate(name, synopsis, functionlist, refidlist):
		func_file = open(name+'/functions.docbook', "w")
		for func in functionlist:
			sgml = docbook_functions.generate_sgml(func, synopsis)
			link_refids(sgml, refidlist)
			cleanml = sgml.toxml()+'\n'
			func_file.write(cleanml)
		func_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific function
	"""
	def generate_sgml(function, synopsis):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, 'sect2')
		sect2.setAttribute('id', function['id'])

		title = create_append(sect2, 'title')
		create_append_text(title, function['name'])

		# synopsis
		funcsynopsis = create_append(sect2, 'funcsynopsis')
		funcsynopsisinfo = create_append(funcsynopsis, 'funcsynopsisinfo')
		create_append_text(funcsynopsisinfo, "#include <"+synopsis+">")

		# prototype
		funcprototype = create_append(funcsynopsis, 'funcprototype')

		dbhtml = create_append_pi(funcsynopsis, 'dbhtml', "funcsynopsis-style='ansi'")

		funcdef = create_append(funcprototype, 'funcdef')
		create_append_text(funcdef, function['return'])
		if function['return'][-1:] != "*":
				# dont add space between * and name
				create_append_text(funcdef, " ")

		func = create_append(funcdef, 'function')
		create_append_text(func, function['name'])

		# add parameter to function definition
		param_num = len(function['param'])
		if param_num == 1 and function['param'][0].is_void():
			void = create_append(funcprototype, 'void')
		else:
			for i in range(0, param_num):
				function['param'][i].dom_append(funcprototype)

		# add help to function
		function['help'].dom_append(sect2)

		return sect2

	# make functions "static"
	generate = staticmethod(generate)
	generate_sgml = staticmethod(generate_sgml)

class docbook_structs:
	"""
	Generate docbook file with informations to all structs
	"""
	def generate(name, structlist, refidlist):
		struct_file = open(name+'/structs.docbook', "w")
		for struct in structlist:
			sgml = docbook_structs.generate_sgml(struct)
			link_refids(sgml, refidlist)
			cleanml = sgml.toxml()+'\n'
			struct_file.write(cleanml)
		struct_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific struct
	"""
	def generate_sgml(struct):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, 'sect2')
		sect2.setAttribute('id', struct['id'])

		title = create_append(sect2, 'title')
		create_append_text(title, 'struct '+struct['name'])

		# add definition of struct
		programlisting = create_append(sect2, 'programlisting')
		create_append_text(programlisting, 'struct '+struct['name']+' {\n')
		for element in struct['elements']:
			element.dom_append(programlisting)
		create_append_text(programlisting, '}')

		# add help to struct
		struct['help'].dom_append(sect2)

		# add list of struct members with their help
		variablelist = create_append(sect2, 'variablelist')
		for element in struct['elements']:
			element.dom_append_help(variablelist)

		# remove empty variablelist
		if len(variablelist.childNodes) == 0:
			sect2.removeChild(variablelist)

		return sect2

	# make functions "static"
	generate = staticmethod(generate)
	generate_sgml = staticmethod(generate_sgml)

class docbook_typedefs:
	"""
	Generate docbook file with informations to all typedefs
	"""
	def generate(name, typedeflist, refidlist):
		typedef_file = open(name+'/typedefs.docbook', "w")
		for typedef in typedeflist:
			sgml = docbook_typedefs.generate_sgml(typedef)
			link_refids(sgml, refidlist)
			cleanml = sgml.toxml()+'\n'
			typedef_file.write(cleanml)
		typedef_file.close()

	"""
	Generate docbook sect2 dom with informations to a specific typedef
	"""
	def generate_sgml(typedef):
		sgml = xml.dom.minidom.Document()
		sect2 = create_append(sgml, 'sect2')
		sect2.setAttribute('id', typedef['id'])

		title = create_append(sect2, 'title')
		create_append_text(title, 'typedef '+typedef['name'])

		# add definition of typedef
		programlisting = create_append(sect2, 'programlisting')
		create_append_text(programlisting, typedef['definition'])

		# add help to typedef
		typedef['help'].dom_append(sect2)

		return sect2

	# make functions "static"
	generate = staticmethod(generate)
	generate_sgml = staticmethod(generate_sgml)


def manpage_header(root, name, refid, mannum, ref_name, ref_namediv, synopsisinfo):
	refentry = create_append(root, 'refentry')
	refentry.setAttribute('id', cleanup_stringbegin(refid))

	refmeta = create_append(refentry, 'refmeta')

	refentrytitle = create_append(refmeta, 'refentrytitle')
	create_append_text(refentrytitle, name)

	manvolnum = create_append(refmeta, 'manvolnum')
	create_append_text(manvolnum, mannum)

	refnamediv = create_append(refentry, 'refnamediv')

	refname = create_append(refnamediv, 'refname')
	create_append_text(refname, cleanup_stringbegin(ref_name))
	refpurpose = create_append(refnamediv, 'refpurpose')
	create_append_text(refpurpose, ref_namediv)

	# synopsis
	refsynopsisdiv = create_append(refentry, 'refsynopsisdiv')
	funcsynopsis = create_append(refsynopsisdiv, 'funcsynopsis')
	funcsynopsisinfo = create_append(funcsynopsis, 'funcsynopsisinfo')
	create_append_text(funcsynopsisinfo, synopsisinfo)

	return (refentry, funcsynopsis)

class manpage_functions:
	"""
	Generate manpage docbook file with informations to functions
	"""
	def generate(synopsis, functionlist):
		for func in functionlist:
			func_file = open('./manpages/man3/'+cleanup_stringbegin(func['name'])+'.sgml', "w")
			func_file.write('<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"\n'+
					'"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd">\n')
			sgml = manpage_functions.generate_sgml(func, synopsis)
			sgml.writexml(func_file)
			func_file.close()

	"""
	Generate manpage docbook dom with informations to a specific function
	"""
	def generate_sgml(function, synopsis):
		sgml = xml.dom.minidom.Document()

		(refentry, funcsynopsis) = manpage_header(sgml, function['name'], function['id'], '3', function['name'], function['brief'].strip(), "#include <"+synopsis+">")

		# prototype
		funcprototype = create_append(funcsynopsis, 'funcprototype')

		dbhtml = create_append_pi(funcsynopsis, 'dbhtml', "funcsynopsis-style='ansi'")

		funcdef = create_append(funcprototype, 'funcdef')
		create_append_text(funcdef, function['return'])
		if function['return'][-1:] != "*":
				# dont add space between * and name
				create_append_text(funcdef, " ")

		func = create_append(funcdef, 'function')
		create_append_text(func, function['name'])

		# add parameter to function definition
		param_num = len(function['param'])
		if param_num == 1 and function['param'][0].is_void():
			void = create_append(funcprototype, 'void')
		else:
			for i in range(0, param_num):
				function['param'][i].dom_append(funcprototype, "\t")

		# add help to function
		refsect1 = create_append(refentry, 'refsect1')
		title = create_append(refsect1, 'title')
		create_append_text(title, "Description")
		function['help'].dom_append(refsect1)

		return refentry

	# make functions "static"
	generate = staticmethod(generate)
	generate_sgml = staticmethod(generate_sgml)

class manpage_structs:
	"""
	Generate manpage docbook file with informations to all structs
	"""
	def generate(synopsis, structlist):
		for func in structlist:
			func_file = open('./manpages/man9/'+cleanup_stringbegin(func['name'])+'.sgml', "w")
			func_file.write('<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"\n'+
					'"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd">\n')
			sgml = manpage_structs.generate_sgml(func, synopsis)
			sgml.writexml(func_file)
			func_file.close()

	"""
	Generate manpage docbook dom with informations to a specific struct
	"""
	def generate_sgml(struct, synopsis):
		sgml = xml.dom.minidom.Document()

		(refentry, funcsynopsis) = manpage_header(sgml, struct['name'], struct['id'], '9', struct['name'], "", "#include <"+synopsis+">")

		# add definition of struct
		refsect1 = create_append(refentry, 'refsect1')
		title = create_append(refsect1, 'title')
		create_append_text(title, "Structure Members")
		
		programlisting = create_append(refsect1, 'programlisting')
		create_append_text(programlisting, 'struct '+struct['name']+' {\n')
		for element in struct['elements']:
			element.dom_append(programlisting)
		create_append_text(programlisting, '}')

		# add help to struct
		refsect1 = create_append(refentry, 'refsect1')
		title = create_append(refsect1, 'title')
		create_append_text(title, "Description")
		struct['help'].dom_append(refsect1)

		# add list of struct members with their help
		variablelist = create_append(refsect1, 'variablelist')
		for element in struct['elements']:
			element.dom_append_help(variablelist)

		# remove empty variablelist
		if len(variablelist.childNodes) == 0:
			refsect1.removeChild(variablelist)

		return refentry

	# make functions "static"
	generate = staticmethod(generate)
	generate_sgml = staticmethod(generate_sgml)

if __name__ == '__main__':
	main()
