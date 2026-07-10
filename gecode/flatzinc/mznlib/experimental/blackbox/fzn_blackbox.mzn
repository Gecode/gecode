include "blackbox_annotations.mzn";

% Value blackbox. The output arrays are constrained to the values returned by
% the selected blackbox when all input variables are fixed. If both input arrays
% are empty, the blackbox is evaluated once at posting.
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
