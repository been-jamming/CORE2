#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dictionary.h"
#include "types.h"
#include "commands.h"

static parse_context *current_context;
static parse_context *root_context;
extern dictionary *definitions;

/*
void new_context(void){
	parse_context *next_context;

	next_context = malloc(sizeof(parse_context));
	next_context->variables = malloc(sizeof(dictionary));
	next_context->definitions = malloc(sizeof(dictionary));
	next_context->goal = NULL;
	next_context->parent = current_context;

	next_context->variables = create_dictionary(NULL);
	next_context->definitions = create_dictionary(NULL);

	current_context = next_context;
}
*/

void free_definition(definition *def){
	int k;

	free(def->name);

	free_type(def->definition_type);
	
	free(def);
}

void free_definition_void(void *def){
	free_definition(def);
}

void free_variable(variable *var){
	free(var->name);

	if(var->option == VARIABLE_USER_DEFINED || var->option == VARIABLE_PRIMITIVE){
		free_type(var->variable_type);
	}

	free(var);
}

void free_variable_void(void *var){
	free_variable(var);
}

void return_context(void){
	parse_context *next_context;
	free_dictionary(current_context->variables, free_variable_void);
	free_dictionary(current_context->definitions, free_definition_void);
	free(current_context->variables);
	free(current_context->definitions);
	if(current_context->goal){
		free_type(current_context->goal);
	}

	next_context = current_context->parent;
	free(current_context);
	current_context = next_context;
}

int is_whitespace(char c){
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int check_command_name(char *source, char *command){
	int command_length;

	command_length = strlen(command);

	if(memcmp(source, command, sizeof(char)*command_length)){
		return 0;
	}

	return is_whitespace(source[command_length]);
}

int get_identifier(char **c, char **identifier){
	int identifier_length;

	identifier_length = get_identifier_name_length(*c);

	if(identifier_length == 0){
		return 0;
	}

	*identifier = malloc(sizeof(char)*(identifier_length + 1));
	memcpy(*identifier, *c, sizeof(char)*identifier_length);
	(*identifier)[identifier_length] = '\0';

	*c += identifier_length;
	return identifier_length;
}

unsigned int get_type_num_arguments(type *inp){
	unsigned int output = 0;

	while(inp && inp->option == TYPE_VAR_DEPEND){
		output++;
		inp = inp->output_type;
	}

	return output;
}

void parse_define(char **c, parse_context *context){
	char *identifier;
	definition *new_definition;
	int k;
	unsigned int num_arguments;
	variable *var;

	*c += strlen("define");
	skip_whitespace(c);

	if(!get_identifier(c, &identifier)){
		fprintf(stderr, "Error: expected identifier\n");
		exit(1);
	}

	new_definition = malloc(sizeof(definition));
	new_definition->option = DEFINITION_USER_DEFINED;
	new_definition->name = identifier;

	skip_whitespace(c);

	if(**c != ':'){
		fprintf(stderr, "Error: expected ':'\n");
		exit(1);
	}

	++*c;

	new_definition->definition_type = parse_type(c, 0);

	skip_whitespace(c);
	
	if(**c != ';'){
		fprintf(stderr, "Error: expected ';'\n");
		exit(1);
	}

	++*c;

	new_definition->num_arguments = get_type_num_arguments(new_definition->definition_type);
	write_dictionary(definitions, identifier, new_definition, 0);
}

extern int main2(int argc, char **argv);

int main(int argc, char **argv){
	char *c = "define bla: $a:Nat $b:Nat Test(a, b) & Test(a, a);";

	main2(argc, argv);
	parse_define(&c, NULL);

	return 0;
}
