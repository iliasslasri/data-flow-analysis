# Iliass Lasri



## Dead Code Analysis
    - The abstract domain is { DEAD, REACHABLE }.
    - Lattice : 
    - the join operation is an operation that sees if on of the inputs is REACHABLE, the instruction or block is REACHABLE, else DEAD.
        a join b =  REACHABLE iff b or a is REACHABLE.
                    DEAD if not

## Combined Dead Code and Constant Analysis 

    To run this analysis, make sure we don't compile '''WhileValueRangeAnalysis.cc''' (comment it in the CMakeLists.txt), then do the same for '''WhileConstantDeadAnalysis.cc''' to run the Value Range Analysis.