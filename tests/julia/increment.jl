str_firstname = "adrien"

function increment(sck_in::Array{UInt8}, sck_out::Array{UInt8}, cst_wait_time_ns::UInt32, rnt_frame_id::UInt32, rnt_n_frames_per_wave::UInt32)::Int32
    # println("coucou ", str_firstname, " :-), rnt_frame_id = ", rnt_frame_id, ", rnt_n_frames_per_wave = ", rnt_n_frames_per_wave)

    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
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
    println("coucou ", cst_firstname, " :-)")

    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
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
    if cst_wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
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