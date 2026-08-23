#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "dictionary.h"
#include "types.h"

dictionary *definitions;
dictionary *variables;
dictionary *parse_variables;

void skip_whitespace(char **c){
	while(**c == ' ' || **c == '\t' || **c == '\n' || **c == '\r'){
		++*c;
	}
}

int is_alpha(char c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_numeric(char c){
	return c >= '0' && c <= '9';
}

int is_identifier_char(char c){
	return is_alpha(c) || is_numeric(c) || c == '_';
}

int get_identifier_name_length(char *c){
	int length;

	if(is_alpha(*c)){
		length = 1;
	} else {
		return 0;
	}

	while(length < INT_MAX && is_identifier_char(c[length])){
		length++;
	}

	return length;
}

static type *duplicate_type(type *input){
	unsigned int k;
	type *output;

	output = malloc(sizeof(type));
	output->num_bound_vars = input->num_bound_vars;

	switch(input->option){
		case PRODUCT:
		case SUM:
			output->input_type = duplicate_type(input->input_type);
			output->output_type = duplicate_type(input->output_type);
			break;
		case AND:
		case OR:
			output->type0 = duplicate_type(input->type0);
			output->type1 = duplicate_type(input->type1);
			break;
		case DEFINITION:
			output->definition_data = input->definition_data;
			output->arguments = malloc(sizeof(type)*input->definition_data->num_arguments);
			for(k = 0; k < input->definition_data->num_arguments; k++){
				if(input->arguments[k]->option == VARIABLE_BOUND){
					output->arguments[k] = malloc(sizeof(variable));
					output->arguments[k]->option = VARIABLE_BOUND;
					output->arguments[k]->name = malloc(sizeof(char)*(strlen(input->arguments[k]->name) + 1));
					strcpy(output->arguments[k]->name, input->arguments[k]->name);
					output->arguments[k]->variable_type = duplicate_type(input->arguments[k]->variable_type);
				} else {
					output->arguments[k] = input->arguments[k];
				}
			}
			break;
	}

	output->option = input->option;

	return output;
}

static int types_identical(type *input0, type *input1){
	int k;

	if(input0->option != input1->option || input0->num_bound_vars != input1->num_bound_vars){
		return 0;
	}

	switch(input0->option){
		case PRODUCT:
		case SUM:
			return types_identical(input0->input_type, input1->input_type) && types_identical(input0->output_type, input1->output_type);
		case AND:
		case OR:
			return types_identical(input0->type0, input1->type0) && types_identical(input0->type1, input1->type1);
		case DEFINITION:
			if(input0->definition_data != input1->definition_data){
				return 0;
			}

			for(k = 0; k < input0->definition_data->num_arguments; k++){
				if(input0->arguments[k]->option != input1->arguments[k]->option){
					return 0;
				}
				if(input0->arguments[k]->option == VARIABLE_BOUND){
					if(input0->arguments[k]->variable_id != input1->arguments[k]->variable_id){
						return 0;
					}
				} else {
					if(input0->arguments[k] != input1->arguments[k]){
						return 0;
					}
				}
			}

			return 1;
	}
}

static variable *get_type_variable(char **c){
	int identifier_length;
	char *identifier;
	variable *named_variable;

	skip_whitespace(c);
	identifier_length = get_identifier_name_length(*c);
	if(identifier_length <= 0){
		fprintf(stderr, "Error: expected identifier\n");
		exit(1);
	}

	identifier = malloc(sizeof(char)*(identifier_length + 1));
	memcpy(identifier, *c, sizeof(char)*identifier_length);
	identifier[identifier_length] = '\0';

	named_variable = read_dictionary(*parse_variables, identifier, 0);

	if(!named_variable){
		fprintf(stderr, "Error: unknown variable '%s'\n", identifier);
		exit(1);
	}

	free(identifier);

	*c += identifier_length;
	return named_variable;
}

static type *parse_type_value(char **c, unsigned int num_bound_vars){
	type *output;
	type *input_type;
	type *output_type;
	variable *var;
	int identifier_length;
	int is_product;
	int k;
	char *identifier;
	char *saved_pos;
	definition *named_def;
	
	skip_whitespace(c);
	identifier_length = get_identifier_name_length(*c);
	if(identifier_length > 0){
		identifier = malloc(sizeof(char)*(identifier_length + 1));
		memcpy(identifier, *c, sizeof(char)*identifier_length);
		identifier[identifier_length] = '\0';

		named_def = read_dictionary(*definitions, identifier, 0);

		if(named_def){
			*c += identifier_length;
			skip_whitespace(c);
		} else {
			fprintf(stderr, "Error: unknown definition '%s'\n", identifier);
			exit(1);
		}

		output = malloc(sizeof(type));
		output->num_bound_vars = num_bound_vars;
		output->option = DEFINITION;
		output->definition_data = named_def;
		if(named_def->num_arguments > 0){
			output->arguments = malloc(sizeof(type *));
		} else {
			output->arguments = NULL;
		}

		if(**c == '('){
			saved_pos = *c;
			++*c;
			skip_whitespace(c);
			//We wish to allow stray () or parentheses around a type after a definition which depends on no arguments
			if(named_def->num_arguments == 0 && **c != ')'){
				*c = saved_pos;
				return output;
			}
			for(k = 0; k < named_def->num_arguments; k++){
				output->arguments[k] = get_type_variable(c);
				if(!types_identical(named_def->argument_types[k], output->arguments[k]->variable_type)){
					fprintf(stderr, "Error: mismatched arguments\n");
					exit(1);
				}
				skip_whitespace(c);
				if(k + 1 < named_def->num_arguments && **c != ','){
					fprintf(stderr, "Error: expected ','\n");
					exit(1);
				} else if(k + 1 < named_def->num_arguments && **c == ','){
					++*c;
					skip_whitespace(c);
				}
			}

			if(**c != ')'){
				fprintf(stderr, "Error: expected ')'\n");
				exit(1);
			}

			++*c;

			return output;
		} else {
			if(named_def->num_arguments > 0){
				fprintf(stderr, "Error: expected '('\n");
				exit(1);
			}

			return output;
		}
	} else if(**c == '('){
		++*c;
		output = parse_type(c, num_bound_vars);
		skip_whitespace(c);

		if(**c != ')'){
			fprintf(stderr, "Error: expected ')'\n");
			exit(1);
		}

		++*c;
		return output;
	} else if(**c == '*' || **c == '^'){
		is_product = (**c == '*');
		++*c;
		skip_whitespace(c);
		identifier_length = get_identifier_name_length(*c);
		if(identifier_length <= 0){
			fprintf(stderr, "Error: expected identifier\n");
			exit(1);
		}

		identifier = malloc(sizeof(char)*(identifier_length + 1));
		memcpy(identifier, *c, sizeof(char)*identifier_length);
		identifier[identifier_length] = '\0';

		var = malloc(sizeof(variable));
		var->option = VARIABLE_BOUND;
		var->name = identifier;
		
		*c += identifier_length;
		skip_whitespace(c);
		if(**c != ':'){
			fprintf(stderr, "Error: expected ':'\n");
			exit(1);
		}

		++*c;
		skip_whitespace(c);

		input_type = parse_type(c, num_bound_vars);
		var->variable_type = input_type;
		var->variable_id = num_bound_vars;

		write_dictionary(parse_variables, var->name, var, 0);

		skip_whitespace(c);

		output_type = parse_type(c, num_bound_vars + 1);

		output = malloc(sizeof(type));

		output->num_bound_vars = num_bound_vars;
		if(is_product){
			output->option = PRODUCT;
		} else {
			output->option = SUM;
		}
		output->input_type = input_type;
		output->output_type = output_type;

		write_dictionary(parse_variables, var->name, NULL, 0);

		return output;
	}
}

static type *parse_type_recursive(char **c, type *prev_type_value, unsigned int num_bound_vars, int precedence){
	type *output;
	type *type_value;
	char *var_name0;
	char *var_name1;

	skip_whitespace(c);

	if(**c == '|' && precedence <= 1){
		++*c;
		type_value = parse_type_value(c, num_bound_vars);
		type_value = parse_type_recursive(c, type_value, num_bound_vars, 1);

		output = malloc(sizeof(type));
		output->option = OR;
		output->num_bound_vars = num_bound_vars;

		output->type0 = prev_type_value;
		output->type1 = type_value;

		return output;
	} else if(**c == '&' && precedence <= 2){
		++*c;
		type_value = parse_type_value(c, num_bound_vars);
		type_value = parse_type_recursive(c, type_value, num_bound_vars, 2);

		output = malloc(sizeof(type));
		output->option = AND;
		output->num_bound_vars = num_bound_vars;

		output->type0 = prev_type_value;
		output->type1 = type_value;

		return output;
	} else {
		return prev_type_value;
	}
}

type *parse_type(char **c, unsigned int num_bound_vars){
	type *prev_type_value = NULL;
	type *next_type_value;

	next_type_value = parse_type_value(c, num_bound_vars);

	if(!next_type_value){
		return NULL;
	}

	do{
		prev_type_value = next_type_value;
		next_type_value = parse_type_recursive(c, prev_type_value, num_bound_vars, 0);
	} while(next_type_value != prev_type_value);

	return next_type_value;
}

void print_type(type *input_type){
	int k;

	if(!input_type){
		return;
	}

	if(input_type->option == PRODUCT || input_type->option == SUM){
		if(input_type->option == PRODUCT)
			printf("*%u:(", input_type->num_bound_vars);
		else
			printf("^%u:(", input_type->num_bound_vars);
		print_type(input_type->input_type);
		printf(")(");
		print_type(input_type->output_type);
		printf(")");
	} else if(input_type->option == AND || input_type->option == OR){
		printf("(");
		print_type(input_type->type0);
		if(input_type->option == AND)
			printf(")&(");
		else
			printf(")|(");
		print_type(input_type->type1);
		printf(")");
	} else if(input_type->option == DEFINITION){
		printf("%s", input_type->definition_data->name);
		if(input_type->definition_data->num_arguments > 0){
			printf("(");
			for(k = 0; k < input_type->definition_data->num_arguments; k++){
				if(input_type->arguments[k]->option == VARIABLE_BOUND){
					printf("%u", input_type->arguments[k]->variable_id);
				} else {
					printf("%s", input_type->arguments[k]->name);
				}

				if(k + 1 < input_type->definition_data->num_arguments){
					printf(",");
				}
			}
			printf(")");
		}
	}
}

int main(int argc, char **argv){
	type *parsed_type;
	char *type_string = "Test(hi, hi)";
	char *type_string2 = "*C:True(True)";
	char *type_string3 = "*D:True(True)";
	char *type_string4 = "*A:True(True)";
	definition true_definition;
	definition false_definition;
	definition test_definition;
	variable hi_variable;
	
	definitions = malloc(sizeof(dictionary));
	*definitions = create_dictionary(NULL);
	parse_variables = malloc(sizeof(dictionary));
	*parse_variables = create_dictionary(NULL);

	true_definition = (definition) {.option = DEFINITION_PRIMITIVE, .name = "True", .num_arguments = 0, .argument_types = NULL};
	false_definition = (definition) {.option = DEFINITION_PRIMITIVE, .name = "False", .num_arguments = 0, .argument_types = NULL};

	write_dictionary(definitions, "True", &true_definition, 0);
	write_dictionary(definitions, "False", &false_definition, 0);

	test_definition = (definition) {.option = DEFINITION_PRIMITIVE, .name = "Test", .num_arguments = 2, .argument_types = malloc(sizeof(type *)*2)};
	test_definition.argument_types[0] = parse_type(&type_string2, 0);
	test_definition.argument_types[1] = parse_type(&type_string3, 0);

	write_dictionary(definitions, "Test", &test_definition, 0);

	hi_variable = (variable) {.option = VARIABLE_PRIMITIVE, .name = "hi", .variable_type = parse_type(&type_string4, 0)};

	write_dictionary(parse_variables, "hi", &hi_variable, 0);

	parsed_type = parse_type(&type_string, 0);
	print_type(parsed_type);
	printf("\n");
	
	return 0;
}

