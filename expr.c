#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "expr.h"

extern dictionary *variables;

expr *parse_expression_variable(char **c){
	int identifier_length;
	variable *var;
	char *identifier;
	expr *output;

	skip_whitespace(c);

	identifier_length = get_identifier_name_length(*c);
	if(identifier_length <= 0){
		fprintf(stderr, "Error: expected identifier\n");
		exit(1);
	}

	identifier = malloc(sizeof(char)*(identifier_length + 1));
	memcpy(identifier, *c, sizeof(char)*identifier_length);
	identifier[identifier_length] = '\0';

	var = read_dictionary(*variables, identifier, 0);

	if(!var){
		fprintf(stderr, "Error: unknown variable '%s'\n", identifier);
	}

	free(identifier);

	*c += identifier_length;

	output = malloc(sizeof(expr));
	output->option = EXPR_VARIABLE;
	output->variable = var;

	return output;
}

expr *parse_expression_value(char **c){
	expr *output;
	expr *next_output;
	expr *apply_input;

	skip_whitespace(c);

	if(**c == '('){
		++*c;
		output = parse_expression(c);
		skip_whitespace(c);
		if(**c != ')'){
			fprintf(stderr, "Error: expected matching ')'\n");
			exit(1);
		}
		++*c;
	} else {
		output = parse_expression_variable(c);
		skip_whitespace(c);

		while(**c == '('){
			++*c;
			apply_input = parse_expression(c);
			skip_whitespace(c);
			if(**c != ')'){
				fprintf(stderr, "Error: expected matching ')'\n");
				exit(1);
			}
			++*c;
			skip_whitespace(c);

			next_output = malloc(sizeof(expr));
			next_output->option = EXPR_APPLY;
			next_output->expr0 = output;
			next_output->expr1 = apply_input;
			output->parent = next_output;
			apply_input->parent = next_output;

			output = next_output;
		}

		return output;
	}
}
