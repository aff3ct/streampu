mutable struct StepState
    step::UInt8
    calls::UInt32
end

mutable struct SharedBufferState
    buf::Vector{UInt8}
end

