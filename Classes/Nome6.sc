Nome6 : MultiOutUGen {
    *ar { |ctlMatrix, modMatrix, oversample = 2|
        var zeroMatrix;

        ctlMatrix = ctlMatrix ?? {
            [0, 0, 0, 0, 0, 0] ! 6
        };

        if(ctlMatrix.size != 6) {
            Error("Nome6 ctlMatrix must contain 6 operator rows").throw;
        };

        // FM7-compatible rows [freq, phase, amp] are accepted.
        // Full rows are [freq, phase, amp, m, family, bandwidth].
        ctlMatrix = ctlMatrix.collect { |row|
            if(row.size == 3) {
                row ++ [0, 0, 0]
            } {
                if(row.size == 6) {
                    row
                } {
                    Error(
                        "Nome6 operator rows must have 3 (FM7-style) or 6 fields"
                    ).throw;
                }
            }
        };

        zeroMatrix = {
            Array.fill(6, { Array.fill(6, 0.0) })
        };

        modMatrix = modMatrix ?? { zeroMatrix.value };

        if(modMatrix.size != 6 or: { modMatrix.any { |row| row.size != 6 } }) {
            Error("Nome6 modMatrix must be 6x6").throw;
        };

        if([1, 2, 4].includes(oversample).not) {
            Error("Nome6 oversample must be 1, 2, or 4").throw;
        };

        ^this.multiNewList(
            ['audio']
            ++ ctlMatrix.flat
            ++ modMatrix.flat
            ++ [oversample]
        )
    }

    *numOperators { ^6 }
    *controlArraySize { ^6 }
    *modArraySize { ^36 }

    init { |...theInputs|
        inputs = theInputs;
        ^this.initOutputs(6, rate)
    }
}
