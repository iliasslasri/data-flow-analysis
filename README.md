# Iliass Lasri



## Dead Code Analysis
    - The abstract domain is { DEAD, REACHABLE }.
    - Lattice : 
    - the join operation is an operation that sees if on of the inputs is REACHABLE, the instruction or block is REACHABLE, else DEAD.
        a join b =  REACHABLE iff b or a is REACHABLE.
                    DEAD if not

## Combined Dead Code and Constant Analysis 