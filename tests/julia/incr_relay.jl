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
