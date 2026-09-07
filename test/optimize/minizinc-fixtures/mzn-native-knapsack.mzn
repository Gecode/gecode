% Integer 0..1 variables and the compiler-introduced objective must reach
% automatic exact knapsack DP after the safe objective equality is eliminated.
array[1..6] of var 0..1: take;
array[1..6] of int: weight = [3,3,2,2,4,1];
array[1..6] of int: profit = [5,4,3,3,6,1];
constraint sum(i in 1..6)(weight[i]*take[i]) <= 8;
var int: value = sum(i in 1..6)(profit[i]*take[i]);
solve maximize value;
output [show([take[i] | i in 1..6] ++ [value]), "\n"];
