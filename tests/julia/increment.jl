str_firstname = "adrien"

function increment(sck_in::Array{UInt8}, sck_out::Array{UInt8}, wait_time_ns::UInt32)::Int32
    # println("coucou ", str_firstname, " :-)")

    if wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
    end

    if wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function increment1(sck_in::Array{UInt8}, sck_out::Array{UInt8}, wait_time_ns::UInt32, firstname::String)::Int32
    println("coucou ", firstname, " :-)")

    if wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
    end

    if wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end

function increment2(sck_in, sck_out, wait_time_ns)
    if wait_time_ns > 0
        start_incr = time_ns()
    end

    for n in eachindex(sck_in)
        sck_out[n] = sck_in[n] + 1
    end

    if wait_time_ns != 0
        stop_incr = time_ns()
        elapse_incr = stop_incr - start_incr

        while elapse_incr < wait_time_ns
            elapse_incr = time_ns() - start_incr
        end
    end

    return 0
end