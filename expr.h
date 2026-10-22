#include "types.h"

enum expr_option{
	EXPR_CONSTANT,
	EXPR_APPLY_PRODUCT,
	EXPR_APPLY_SUM,
	EXPR_PRIMITIVE_FUNC,
	EXPR_OPERATION,
	EXPR_VARIABLE,
	EXPR_CONSTRUCT
};

typedef struct expr expr;
typedef struct expr_operation expr_operation;
typedef struct expr_primitive_func expr_primitive_func;
typedef struct expr_constant expr_constant;

struct expr{
	enum expr_option option;
	char *text_origin;
	expr *parent;

	union{
		//EXPR_APPLY_PRODUCT
		//EXPR_APPLY_SUM
		struct{
			expr *expr0;
			union{
				expr *expr1;
				variable *target_variable;
			};
		};
		//EXPR_OPERATION
		struct{
			expr *expr0;
			expr *expr1;
			expr_operation *operation;
		};
		//EXPR_PRIMITIVE_FUNC
		struct{
			char *name;
			expr_primitive_func *primitive;
		};
		//EXPR_CONSTANT
		expr_constant *constant;
		//EXPR_VARIABLE
		variable *expr_variable;
	};
};

//TODO
struct expr_operation{
	char *text;
};

//TODO
struct expr_primitive_func{

};

//TODO
struct expr_constant{

};


