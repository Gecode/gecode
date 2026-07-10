include "blackbox_annotations.mzn";

% Bounds blackbox. The integer and float variables are encoded as lower/upper
% bound pairs, and the returned pairs narrow those same variables. flat_reason
% is the MiniZinc explanation encoding used to select which variable bounds can
% reschedule the blackbox. If it selects no dependencies, the blackbox is
% evaluated once at posting.
predicate fzn_blackbox_bounds(
	array[int] of var int: int_input,
	array[int] of var float: float_input,
	array[int] of int: flat_reason
) = gecode_blackbox_bounds(int_input, float_input, flat_reason);

predicate gecode_blackbox_bounds(
	array[int] of var int: int_input,
	array[int] of var float: float_input,
	array[int] of int: flat_reason
);
