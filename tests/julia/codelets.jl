str_firstname = "adrien"

function increment(sck_in::Array{UInt8}, sck_out::Array{UInt8}, cst_wait_time_ns::UInt32, rnt_frame_id::UInt32, rnt_n_frames_per_wave::UInt32)::Int32
    # Use 'Core.println' instead of 'Base.println' because 'Base.println' relies on libuv asynchronous I/O and task
    # switching, which can deadlock or fail on external C++ threads adopted via 'jl_adopt_thread()'.
    # Core.println("coucou ", str_firstname, " :-), rnt_frame_id = ", rnt_frame_id, ", rnt_n_frames_per_wave = ", rnt_n_frames_per_wave)

    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        # Use 'one(eltype(sck_in))' instead of '1' (Int64) to avoid type promotion to Int64 and
        # per-element InexactError checks when converting back to smaller integer types (e.g. UInt8/Int8).
        sck_out[n] = sck_in[n] + one(eltype(sck_in))
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function increment1(sck_in::Array{UInt8}, sck_out::Array{UInt8}, cst_wait_time_ns::UInt32, cst_firstname::String, rnt_frame_id::UInt32, rnt_n_frames_per_wave::UInt32)::Int32
    # Use 'Core.println' instead of 'Base.println' because 'Base.println' relies on libuv asynchronous I/O and task
    # switching, which can deadlock or fail on external C++ threads adopted via 'jl_adopt_thread()'.
    Core.println("coucou ", cst_firstname, " :-)")

    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        # Use 'one(eltype(sck_in))' instead of '1' (Int64) to avoid type promotion to Int64 and
        # per-element InexactError checks when converting back to smaller integer types (e.g. UInt8/Int8).
        sck_out[n] = sck_in[n] + one(eltype(sck_in))
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function increment2(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    # Use 'Core.println' instead of 'Base.println' because 'Base.println' relies on libuv asynchronous I/O and task
    # switching, which can deadlock or fail on external C++ threads adopted via 'jl_adopt_thread()'.
    # Core.println("coucou ", str_firstname, " :-), rnt_frame_id = ", rnt_frame_id, ", rnt_n_frames_per_wave = ", rnt_n_frames_per_wave)

    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        # Use 'one(eltype(sck_in))' instead of '1' (Int64) to avoid type promotion to Int64 and
        # per-element InexactError checks when converting back to smaller integer types (e.g. UInt8/Int8).
        sck_out[n] = sck_in[n] + one(eltype(sck_in))
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function incrementf(sck_fwd, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_fwd)
        # Use 'one(eltype(sck_fwd))' instead of '1' (Int64) to avoid type promotion to Int64 and
        # per-element InexactError checks when converting back to smaller integer types (e.g. UInt8/Int8).
        sck_fwd[n] = sck_fwd[n] + one(eltype(sck_fwd))
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

# Allocate a temporary Vector on the Julia heap at each invocation
# to stress the Garbage Collector (GC) and test multi-threaded GC safety / safepoints.
function increment_gc_stress(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    tmp = Vector{eltype(sck_in)}(undef, length(sck_in))
    for n in eachindex(sck_in)
        tmp[n] = sck_in[n] + one(eltype(sck_in))
        sck_out[n] = tmp[n]
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function relay(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_rly = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n]
    end

    if cst_wait_time_ns != 0
        stop_rly = time_ns()
        elapse_rly = stop_rly - start_rly

        while elapse_rly < cst_wait_time_ns
            elapse_rly = time_ns() - start_rly
        end
    end

    return 0
end

# Allocate a temporary Vector on the Julia heap at each invocation
# to stress the Garbage Collector (GC) and test multi-threaded GC safety / safepoints.
function relay_gc_stress(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_rly = time_ns()
    end

    tmp = Vector{eltype(sck_in)}(undef, length(sck_in))
    for n in eachindex(sck_in)
        tmp[n] = sck_in[n]
        sck_out[n] = tmp[n]
    end

    if cst_wait_time_ns != 0
        stop_rly = time_ns()
        elapse_rly = stop_rly - start_rly

        while elapse_rly < cst_wait_time_ns
            elapse_rly = time_ns() - start_rly
        end
    end

    return 0
end

function increment_stateful(stt_step, sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    if rnt_frame_id == 0
        stt_step.calls += one(typeof(stt_step.calls))
    end

    step_val = eltype(sck_in)(stt_step.step)
    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + step_val
    end

    if cst_wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < cst_wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function write_buf(stt_buf, sck_in, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    if cst_wait_time_ns > 0
        start_wr = time_ns()
    end

    frame_len = div(length(sck_in), Int(rnt_n_frames_per_wave))
    offset = Int(rnt_frame_id) * frame_len
    if length(stt_buf.buf) < offset + length(sck_in)
        resize!(stt_buf.buf, offset + length(sck_in))
    end

    for n in eachindex(sck_in)
        stt_buf.buf[offset + n] = sck_in[n] + one(eltype(sck_in))
    end

    if cst_wait_time_ns != 0
        stop_wr = time_ns()
        elapse_wr = stop_wr - start_wr

        while elapse_wr < cst_wait_time_ns
            elapse_wr = time_ns() - start_wr
        end
    end

    return 0
end

function read_buf(stt_buf, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    frame_len = div(length(sck_out), Int(rnt_n_frames_per_wave))
    offset = Int(rnt_frame_id) * frame_len
    for n in eachindex(sck_out)
        sck_out[n] = stt_buf.buf[offset + n]
    end
    return 0
end

