#include "types.h"

struct subtype_stack{
	type *t0;
	type *t1;
	struct subtype_stack *parent;
};

int coerce_type_t1_AND(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0;
	node.t1 = stack->t1->type0;
	node.parent = stack;

	return coerce_type_recursive(&node);
}

int coerce_type_t1_OR(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0;
	node.t1 = stack->t1->type0;
	node.parent = stack;

	if(coerce_type_recursive(&node)){
		return 1;
	} else {
		node.t1 = stack->t1->type1;
		return coerce_type_recursive(&node);
	}
}

int coerce_type_t0_OR(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0->type0;
	node.t1 = stack->t1;
	node.parent = stack;

	return cerce_type_recursive(&node);
}

int coerce_type_t0_AND(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0->type0;
	node.t1 = stack->t1;
	node.parent = stack;

	if(coerce_type_recursive(&node)){
		return 1;
	} else {
		node.t0 = stack->t0->type1;
		return coerce_type_recursive(&node);
	}
}

int coerce_type_definitions(struct subtype_stack *stack){
	int k;
	int result = 0;
	unsigned int num_arguments;
	argument arg0;
	argument arg1;
	coercion_map *coercion_maps_source0;
	coercion_map *coercion_maps_target0;
	coercion_map *coercion_maps_source1;
	coercion_map *coercion_maps_target1;

	if(stack->t0->definition_data != stack->t1->definition_data){
		return 1;
	}

	num_arguments = stack->t0->definition_data->num_arguments;

	coercion_maps_source0 = malloc(sizeof(coercion_map)*num_arguments);
	coercion_maps_target0 = malloc(sizeof(coercion_map)*num_arguments);
	coercion_maps_source1 = malloc(sizeof(coercion_map)*num_arguments);
	coercion_maps_target1 = malloc(sizeof(coercion_map)*num_arguments);

	//Save the prior coercion maps
	for(k = 0; k < num_arguments; k++){
		arg0 = stack->t0->arguments[k];
		arg1 = stack->t1->arguments[k];

		if(arg0.option == ARGUMENT_BOUND){
			coercion_maps_source0[k] = arg0.subtype_source->source;
			coercion_maps_target0[k] = arg0.subtype_source->target;
		}

		if(arg1.option == ARGUMENT_BOUND){
			coercion_maps_source1[k] = arg1.subtype_source->source;
			coercion_maps_target1[k] = arg1.subtype_source->target;
		}
	}

	//Bind new coercion maps and check prior bindings
	for(k = 0; k < stack->t0->definition_data->num_arguments; k++){
		arg0 = stack->t0->arguments[k];
		arg1 = stack->t1->arguments[k];
		if(arg0.option == ARGUMENT_ENVIRONMENT && arg1.option == ARGUMENT_BOUND){
			if(arg1.subtype_source->option != SUM){
				result = 1;
				break;
			}
			if(arg1.subtype_source->source.option == COERCION_MAP_NONE){
				arg1.subtype_source->source.option = COERCION_MAP_ENVIRONMENT;
				arg1.subtype_source->source.coercion_variable = arg0.argument_variable;
			} else if(arg1.subtype_source->source.option == COERCION_MAP_ENVIRONMENT){
				if(arg1.subtype_source->source.coercion_variable != arg0.argument_variable){
					result = 1;
					break;
				}
			} else {
				result = 1;
				break;
			}
		} else if(arg0.option == ARGUMENT_BOUND && arg1.option == ARGUMENT_ENVIRONMENT){
			if(arg0.subtype_source->option != PRODUCT){
				result = 1;
				break;
			}
			if(arg0.subtype_source->target.option == COERCION_MAP_NONE){
				arg0.subtype_source->target.option = COERCION_MAP_ENVIRONMENT;
				arg0.subtype_source->target.coercion_variable = arg1.argument_variable;
			} else if(arg0.subtype_source->target.option == COERCION_MAP_ENVIRONMENT){
				if(arg0.subtype_source->target.coercion_variable != arg1.argument_variable){
					result = 1;
					break;
				}
			} else {
				result = 1;
				break;
			}
		} else if(arg0.option == ARGUMENT_BOUND && arg1.option == ARGUMENT_BOUND){
			
		}
	}

	result = result || seek_next(stack);

	if(result){
		//Restore the prior coercion maps
		for(k = 0; k < num_arguments; k++){
			arg0 = stack->t0->arguments[k];
			arg1 = stack->t1->arguments[k];

			if(arg0.option == ARGUMENT_BOUND){
				arg0.subtype_source->source = coercion_maps_source0[k];
				arg0.subtype_source->target = coercion_maps_target0[k];
			}

			if(arg1.option == ARGUMENT_BOUND){
				arg1.subtype_source->source = coercion_maps_source1[k];
				arg1.subtype_source->target = coercion_maps_target1[k];
			}
		}
	}

	free(coercion_maps_source0);
	free(coercion_maps_target0)
	free(coercion_maps_source1);
	free(coercion_maps_target1)

	return result;
}

int coerce_type_recursive(struct subtype_stack *stack){
	if(stack->t1->option == AND){
		return coerce_type_t1_AND(stack);
	} else if(stack->t1->option == OR){
		return coerce_type_t1_OR(stack);
	} else if(stack->t0->option == OR){
		return coerce_type_t0_OR(stack);
	} else if(stack->t0->option == AND){
		return coerce_type_t0_AND(stack);
	} else if(stack->t0->option == DEFINITION && stack->t1->option == DEFINITION){
		return coerce_type_definitions(stack);
	}
}

//Coerce type t0 into type t1
//returns 1 on error
int coerce_type(type *t0, type *t1){
	int result;
	struct subtype_stack root;

	root.parent = NULL;
	root.t0 = t0;
	root.t1 = t1;

	result = coerce_type_recursive(&root);

	if(result){
		result = check_coercion(t0, t1);
	}

	return result;
}
