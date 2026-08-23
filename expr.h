#include "types.h"

enum expr_option{
	EXPR_CONSTANT,
	EXPR_APPLY,
	EXPR_PRIMITIVE_FUNC,
	EXPR_OPERATION
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
		//EXPR_APPLY
		struct{
			expr *expr0;
			expr *expr1;
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
	};
};

//TODO
struct expr_operation{

};

//TODO
struct expr_primitive_func{

};

//TODO
struct expr_constant{

};


