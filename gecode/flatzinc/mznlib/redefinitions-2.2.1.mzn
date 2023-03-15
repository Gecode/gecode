predicate gecode_int_pow(var int: x, int: y, var int: z);

predicate int_pow_fixed(var int: x, int: y, var int: z) =
  if y = 0 then
    z = 1
  elseif y = 1 then
    z = x
  else
    if y >= 0 then
      gecode_int_pow(x, y, z)
    else
      z = 1 div pow(x, -y)
    endif
  endif;
