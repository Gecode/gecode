% Multiple weighted rows avoid a single-knapsack special case. Boolean originals
% keep binary reliability and Hamming neighborhoods eligible after flattening.
array[1..6] of var bool: take;
array[1..6] of int: weight = [3,3,2,2,4,1];
array[1..6] of int: profit = [5,4,3,3,6,1];
constraint sum(i in 1..6)(weight[i]*bool2int(take[i])) <= 8;
constraint 3*bool2int(take[1])+3*bool2int(take[2]) <= 5;
constraint bool2int(take[3])+bool2int(take[4])+bool2int(take[5]) <= 2;
var int: value = sum(i in 1..6)(profit[i]*bool2int(take[i]));
solve maximize value;
output [show([bool2int(take[i]) | i in 1..6] ++ [value]), "\n"];
