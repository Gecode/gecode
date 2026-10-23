include "fzn_cumulatives_decomp.mzn";

predicate fzn_cumulatives(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: r,
  array[int] of var int: m,
  array[int] of var int: b,
  bool: upper,
  int: min_m,
) =
  if is_fixed(b) then
    % Propagator only supports fixed bounds
    let {
      % Match the implicit integer bounds used by the legacy registry binding.
      array[int] of var -2147483646 .. 2147483646: end = [s[i] + d[i] | i in index_set(s)];
    } in
    gecode_cumulatives_flex(
      s,
      d,
      end,
      r,
      [m_i - min_m | m_i in m],
      fix(b),
      upper,
    )
  else
    fzn_cumulatives_decomp(s, d, r, m, b, upper)
  endif;

predicate gecode_cumulatives(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: r,
  array[int] of var int: m,
  array[int] of int: b,
  bool: upper,
);

predicate gecode_cumulatives_flex(
  array[int] of var int: s,
  array[int] of var int: d,
  array[int] of var int: e,
  array[int] of var int: r,
  array[int] of var int: m,
  array[int] of int: b,
  bool: upper,
);
