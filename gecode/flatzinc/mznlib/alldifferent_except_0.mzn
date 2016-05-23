include "global_cardinality_low_up_closed.mzn";

/**  @group globals.alldifferent
  Constrain the array of integers \a vs to be all different except those
  elements that are assigned the value 0.
*/
predicate alldifferent_except_0(array[int] of var int: vs) =
    if length(vs)==0 then true
    else
    let {
      int: l = lb_array(vs);
      int: u = ub_array(vs);
    } in
    if l != -infinity /\ u != infinity then
    global_cardinality_low_up_closed(vs, [i | i in l..u],
                                     [0 | i in l..u],
                                     [if i==0 then length(vs) else 1 endif | i in l..u]
                                     )
    else
    forall(i, j in index_set(vs) where i < j) (
       (vs[i] != 0 /\ vs[j] != 0) -> vs[i] != vs[j]
    )
    endif
    endif;
