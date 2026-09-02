enum type_option{
	PRODUCT,
	SUM,
	AND,
	OR,
	DEFINITION
};

enum definition_option{
	DEFINITION_USER_DEFINED,
	DEFINITION_PRIMITIVE,
	DEFINITION_BOUND
};

enum variable_option{
	VARIABLE_USER_DEFINED,
	VARIABLE_PRIMITIVE,
	VARIABLE_BOUND
};

enum argument_option{
	ARGUMENT_BOUND,
	ARGUMENT_ENVIRONMENT,
	ARGUMENT_CONSTANT
};

typedef struct type type;

typedef struct definition definition;

typedef struct variable variable;

typedef struct argument argument;

struct type{
	enum type_option option;
	unsigned int num_bound_vars;

	union{
		//PRODUCT, SUM
		struct{
			type *input_type;
			type *output_type;
			//Reserved for type duplication
			type *duplication_target;
		};

		//AND, OR
		struct{
			type *type0;
			type *type1;
		};

		//DEFINITION
		struct{
			definition *definition_data;
			argument *arguments;
		};
	};

	type *parent;

	//Reserved for type coercion
	type *source;
	type *target;
};

struct argument{
	enum argument_option option;

	union{
		//ARGUMENT_BOUND
		type *subtype_source;
		//ARGUMENT_ENVIRONMENT
		variable *argument_variable;
		//ARGUMENT_CONSTANT
		//TODO
		
	};
};

struct variable{
	enum variable_option option;

	char *name;
	union{
		type *subtype_source;
		type *variable_type;
	};
};

struct definition{
	enum definition_option option;
	char *name;

	unsigned int num_arguments;
	type **argument_types;
};

void skip_whitespace(char **c);
int is_alpha(char c);
int is_numeric(char c);
int is_identifier_char(char c);
int get_identifier_name_length(char *c);
type *parse_type(char **c, unsigned int num_bound_vars);
void print_type(type *input_type);

