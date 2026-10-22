Nome6 : MultiOutUGen {
    *ar { |ctlMatrix, modMatrix, oversample = 2, noiseColor = 0, noiseCutoff = 8000|
        var zeroMatrix;

        ctlMatrix = ctlMatrix ?? {
            [0, 0, 0, 0, 0, 0, 0, 0, 0] ! 6
        };

        if(ctlMatrix.size != 6) {
            Error("Nome6 ctlMatrix must contain 6 operator rows").throw;
        };

        // Accepted rows:
        // [freq, phase, amp]
        // [freq, phase, amp, m, family, bandwidth]
        // [freq, phase, amp, m, family, bandwidth,
        //  noiseAmp, noisePhase, noiseM]
        ctlMatrix = ctlMatrix.collect { |row|
            if(row.size == 3) {
                row ++ [0, 0, 0, 0, 0, 0]
            } {
                if(row.size == 6) {
                    row ++ [0, 0, 0]
                } {
                    if(row.size == 9) {
                        row
                    } {
                        Error(
                            "Nome6 operator rows must have 3, 6, or 9 fields"
                        ).throw;
                    }
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
            ++ [oversample, noiseColor, noiseCutoff]
        )
    }

    *numOperators { ^6 }
    *controlArraySize { ^9 }
    *modArraySize { ^36 }

    init { |...theInputs|
        inputs = theInputs;
        ^this.initOutputs(6, rate)
    }
}
