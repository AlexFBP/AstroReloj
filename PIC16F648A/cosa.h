typedef union {
	byte	reg;
	struct {
		byte	s0	:2;
		byte	s1	:2;
		byte	s2	:2;
		byte	s3	:2;
	} b2;
	struct {
		byte	r0	:1;
		byte	r1	:1;
		byte	r2	:1;
		byte	r3	:1;
		byte	r4	:1;
		byte	r5	:1;
		byte	r6	:1;
		byte	r7	:1;
	} b1;
} parted ;