cereal: simple serialization of:
	int/uint 16, 32, 64 (int/uint8 = use uchar)
	float32
	float64

Note that serial uses bit operations to make the result
independent of endianness.  As a result, it may be slower.

even if std::numeric_limits<float>.is_iec559(), iirc there
is no endianness guarantee on the resulting bytes of the float
if directly cast to char/uchar.  As such, (for now anyways)
don't bother with these potential optimizations.
