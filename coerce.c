#include "types.h"

struct subtype_stack{
	type *t0;
	type *t1;
	struct subtype_stack *parent;
};

inline int coerce_type_t1_OR(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0;
	node.t1 = stack->t1->type0;
	node.parent = stack;

	if(!coerce_type_recursive(&node)){
		return 0;
	} else {
		node.t1 = stack->t1->type1;
		return coerce_type_recursive(&node);
	}
}

inline int coerce_type_t0_OR(struct subtype_stack *stack){
	struct subtype_stack node;

	node.t0 = stack->t0->type0;
	node.t1 = stack->t1;
	node.parent = stack;

	return coerce_type_recursive(&node);
}

inline int coerce_type_definitions(struct subtype_stack *stack){
	int k;
	int result = 0;
	unsigned int num_arguments;
	argument arg0;
	argument arg1;
	coercion_map *coercion_maps_source0;
	coercion_map *coercion_maps_source1;

	if(stack->t0->definition_data != stack->t1->definition_data){
		return 1;
	}

	num_arguments = stack->t0->definition_data->num_arguments;

	coercion_maps_source0 = malloc(sizeof(coercion_map)*num_arguments);
	coercion_maps_source1 = malloc(sizeof(coercion_map)*num_arguments);

	//Save the prior coercion maps
	for(k = 0; k < num_arguments; k++){
		arg0 = stack->t0->arguments[k];
		arg1 = stack->t1->arguments[k];

		if(arg0.option == ARGUMENT_BOUND){
			coercion_maps_source0[k] = arg0.subtype_source->source;
		}

		if(arg1.option == ARGUMENT_BOUND){
			coercion_maps_source1[k] = arg1.subtype_source->source;
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
			if(arg0.subtype_source->source.option == COERCION_MAP_NONE){
				arg0.subtype_source->source.option = COERCION_MAP_ENVIRONMENT;
				arg0.subtype_source->source.coercion_variable = arg1.argument_variable;
			} else if(arg0.subtype_source->source.option == COERCION_MAP_ENVIRONMENT){
				if(arg0.subtype_source->source.coercion_variable != arg1.argument_variable){
					result = 1;
					break;
				}
			} else {
				result = 1;
				break;
			}
		} else if(arg0.option == ARGUMENT_BOUND && arg1.option == ARGUMENT_BOUND){
			if(arg0.subtype_source->option == PRODUCT && arg1.subtype_source->option == PRODUCT){
				if(arg0.subtype_source->source.option == COERCION_MAP_NONE){
					if(arg1.subtype_source->num_bound_vars >= arg0.subtype_source->source.bound_min &&
					   arg1.subtype_source->num_bound_vars <= arg0.subtype_source->source.bound_max){
						arg0.subtype_source->source.option = COERCION_MAP_BOUND;
						arg0.subtype_source->source.subtype = arg1.subtype_source;
					} else {
						result = 1;
						break;
					}
				} else if(arg0.subtype_source->source.option == COERCION_MAP_BOUND){
					if(arg0.subtype_source->source.subtype != arg1.subtype_source){
						result = 1;
						break;
					}
				} else {
					result = 1;
					break;
				}
			} else if(arg0.subtype_source->option == SUM && arg1.subtype_source->option == SUM){
				if(arg1.subtype_source->source.option == COERCION_MAP_NONE){
					if(arg0.subtype_source->num_bound_vars >= arg1.subtype_source->source.bound_min &&
					   arg0.subtype_source->num_bound_vars <= arg1.subtype_source->source.bound_max){
						arg1.subtype_source->source.option = COERCION_MAP_BOUND;
						arg1.subtype_source->source.subtype = arg0.subtype_source;
					} else {
						result = 1;
						break;
					}
				} else if(arg1.subtype_source->source.option == COERCION_MAP_BOUND){
					if(arg1.subtype_source->source.subtype != arg0.subtype_source){
						result = 1;
						break;
					}
				} else {
					result = 1;
					break;
				}
			}
		} else if(arg0.option == ARGUMENT_ENVIRONMENT && arg1.option == ARGUMENT_ENVIRONMENT){
			if(arg0.argument_variable != arg1.argument_variable){
				result = 1;
				break;
			}
		}
	}

	result = result || seek_next(stack);

	if(result){
		//Restore the prior coercion maps if an error occurred
		for(k = 0; k < num_arguments; k++){
			arg0 = stack->t0->arguments[k];
			arg1 = stack->t1->arguments[k];

			if(arg0.option == ARGUMENT_BOUND){
				arg0.subtype_source->source = coercion_maps_source0[k];
			}

			if(arg1.option == ARGUMENT_BOUND){
				arg1.subtype_source->source = coercion_maps_source1[k];
			}
		}
	}

	free(coercion_maps_source0);
	free(coercion_maps_source1);

	return result;
}

inline int coerce_type_t0_product(struct subtype_stack *stack){
	int bound_min;
	int bound_max;
	type *final_product_t0;
	type *final_product_t1;
	struct subtype_stack node;

	bound_max = stack->t1->num_bound_vars - 1;
	bound_min = stack->t1->num_bound_vars;

	final_product_t1 = stack->t1;

	while(final_product_t1->option == PRODUCT){
		bound_max = final_product_t1->num_bound_vars;
		final_product_t1 = final_product_t1->output_type;
	}

	final_product_t0 = stack->t0;

	while(final_product_t0->option == PRODUCT){
		final_product_t0->source.option = COERCION_MAP_NONE;
		final_product_t0->source.bound_min = bound_min;
		final_product_t0->source.bound_max = bound_max;
		final_product_t0 = final_product_t0->output_type;
	}

	node.t0 = final_product_t0;
	node.t1 = final_product_t1;
	node.parent = stack;

	return coerce_type_recursive(&node);
}

inline int coerce_type_t1_sum(struct subtype_stack *stack){
	int bound_min;
	int bound_max;
	type *final_sum_t0;
	type *final_sum_t1;
	struct subtype_stack node;

	bound_max = stack->t0->num_bound_vars - 1;
	bound_min = stack->t0->num_bound_vars;

	final_sum_t0 = stack->t0;

	while(final_sum_t0->option == SUM){
		bound_max = final_sum_t0->num_bound_vars;
		final_sum_t0 = final_sum_t0->output_type;
	}

	final_sum_t1 = stack->t1;

	while(final_sum_t1->option == SUM){
		final_sum_t1->source.option = COERCION_MAP_NONE;
		final_sum_t1->source.bound_min = bound_min;
		final_sum_t1->source.bound_max = bound_max;
		final_sum_t1 = final_sum_t1->output_type;
	}

	node.t0 = final_sum_t0;
	node.t1 = final_sum_t1;
	node.parent = stack;

	return coerce_type_recursive(&node);
}

int coerce_type_recursive(struct subtype_stack *stack){
	if(stack->t1->option == OR){
		return coerce_type_t1_OR(stack);
	} else if(stack->t0->option == OR){
		return coerce_type_t0_OR(stack);
	} else if(stack->t0->option == DEFINITION && stack->t1->option == DEFINITION){
		return coerce_type_definitions(stack);
	} else if(stack->t0->option == PRODUCT){
		return coerce_type_t0_product(stack);
	} else if(stack->t1->option == SUM){
		return coerce_type_t1_sum(stack);
	} else {
		return 1;
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
