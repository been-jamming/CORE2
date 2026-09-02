#include "types.h"
#include "coerce.h"

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
	variable *var0;
	variable *var1;

	if(stack->t0->definition_data != stack->t1->definition_data){
		return 0;
	}

	for(k = 0; k < stack->t0->definition_data->num_arguments; k++){
		var0 = stack->t0->arguments[k];
		var1 = stack->t1->arguments[k];
		if(var0->option != var1->option){
			return 0;
		}
		if(var0->option == VARIABLE_BOUND){
			//Something here
		}
	}
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
