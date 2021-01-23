goals:
	learn C++/C
	shared libraries/dll, ABI compatibility (pure C interface?) (does that mean there shouldn't be any c++ interface dlls? or have a header wrap the C structs/functions etc into C++ classes?)
	normalized practice


naming convention thoughts:
	tried prefixes:
		example:
			kConstantValue
			mMemberValue
		but not kind of annoying to remember to add them, don't like

	using iSomething for index of Something seems better than just i


	constants: don't use all caps because macro should be all caps?
	this seems to be okay
		type            example
		macros          THIS_IS_A_MACRO
		constants       This_Is_A_Constant
		classes/types   ClassName
		functions       function_name
		variable/member memberOrVariableName

C interface vs C++:
	C++ has namespaces but C doesn't
	name C functions/structs as if namespaced?
	example:
		ns1::ns2::function would wrap ns1__ns2__function

		(replace :: with __ to get C name)
