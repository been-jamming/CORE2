typedef struct parse_context parse_context;

struct parse_context{
	dictionary *variables;
	dictionary *definitions;
	type *goal;
	parse_context *parent;
};

