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

typedef struct type type;

typedef struct definition definition;

typedef struct variable variable;

struct type{
	enum type_option option;
	unsigned int num_bound_vars;

	union{
		//PRODUCT, SUM
		struct{
			type *input_type;
			type *output_type;
		};

		//AND, OR
		struct{
			type *type0;
			type *type1;
		};

		//DEFINITION
		struct{
			definition *definition_data;
			variable **arguments;
		};
	};
};

struct variable{
	enum variable_option option;

	char *name;
	type *variable_type;
	unsigned int variable_id;
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

