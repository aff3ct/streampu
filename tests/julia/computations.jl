function multiple_square_and_multiply(tid, array)
	p = 1392908813
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

function hash_serie(tid, array)
  h::UInt64 = hash(tid)
  for i=1:(1 << 32)
    h = hash(h)
  end
  return h
end

# --------------------------------------------------------

function murmur64(x::UInt64)
  x = xor(x, x >> 33)
  x = x * 0xff51afd7ed558ccd
  x = xor(x, x >> 33)
  x = x * 0xc4ceb9fe1a85ec53
  x = xor(x, x >> 33)
  return x
end

function computation1(tid, array)
  acc = murmur64(UInt64(tid))

  for x in array
    h = murmur64(UInt64(x))
    acc = xor(UInt64(h * acc), acc, h << 16, h >> 48)
  end

  return acc
end
