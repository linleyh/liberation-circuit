
#ifndef H_C_FIX
#define H_C_FIX


#define PROCDEF_BUFFER (GROUP_MAX_MEMBERS*MAX_LINKS*10)
 // I think this should be long enough.

struct procdef_struct
{
// the procdef struct contains a numerical representation of the process definition.
// first, the fixer parses the process definition from the source code and into the procdef
//   OR the procdef is loaded directly from a binary file
// then, the procdef is used to build the actual template.
//  the procdef is checked for validity when used to build a template, so it can contain errors.


 char template_name [TEMPLATE_NAME_LENGTH];

// int class_declared [OBJECT_CLASSES];
// char class_name [OBJECT_CLASSES] [CLASS_NAME_LENGTH];
// class_names may be empty if procdef loaded from binary

// the structure of a procdef reflects the structure of the source code, not the structure of the template data.
 s16b buffer [PROCDEF_BUFFER]; // this is s16b because it is saved directly to template files.
 int buffer_source_line [PROCDEF_BUFFER]; // used to display error messages when writing the procdef to template
 int buffer_length;

// struct procdef_line_struct procdef_line [PROCDEF_LINES];

};


int	fix_template_design_from_scode(void);
int fix_template_design_from_procdef(struct template_struct* target_templ);

int derive_procdef_from_template(struct template_struct* derive_templ);

#endif

