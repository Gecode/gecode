include "blackbox_annotations.mzn";

predicate fzn_blackbox(
	array[int] of var int: int_input,
	array[int] of var float: float_input,
	array[int] of var int: int_output,
	array[int] of var float: float_output
) = gecode_blackbox(int_input, float_input, int_output, float_output);

predicate gecode_blackbox(
	array[int] of var int: int_input,
	array[int] of var float: float_input,
	array[int] of var int: int_output,
	array[int] of var float: float_output
);
