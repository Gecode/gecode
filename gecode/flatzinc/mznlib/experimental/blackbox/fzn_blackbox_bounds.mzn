% Bounds blackbox. Integer variables come first, followed by float variables.
% Each array is encoded as consecutive lower/upper pairs, so variable i uses
% positions 2*i-1 and 2*i in the corresponding backend array. Returned bounds
% are intersected with the current domains; wider returned bounds therefore do
% not widen a Gecode variable.
%
% Every returned interval must contain every value that can occur in a solution
% compatible with the complete input box. Returning an invalid lower or upper
% bound can remove solutions and makes the model unsound.
%
% flat_reason contains one entry for every integer and float variable. Variable
% indices are 1-based over the combined list, with integer variables first. For
% each returned lower and upper bound, its reason must mention every input bound
% on which that result can depend. Gecode subscribes only to variables mentioned
% in these reasons. An omitted dependency can leave a stale result installed
% after an input bound changes and can make search unsound. If no reason mentions
% a dependency, the blackbox is evaluated once while posting.
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
