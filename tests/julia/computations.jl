function multiple_square_and_multiply(p)
  r::Int64 = 0
  for m=1:(1 << 6)
    x::UInt64 = 1
    for i=1:32
      x = (x * x) % p
      x = (x * m) % p
    end
    r = r + x
    if(r > p)
      r = r - p
    end
  end
  return Int32(r)
end

function write_array(sz)
  v = Vector{Int64}(undef, sz)
  for i=1:sz
    v[i] = i
  end
  return v
end
