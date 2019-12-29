__kernel void bandc_kernel( __read_only image2d_t input,
                            __write_only image2d_t output,
                            const float blackLevel,
                            const float whiteLevel )
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int2 outCoord = (int2)(i, j);

    sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
                        CLK_ADDRESS_CLAMP_TO_EDGE  |
                        CLK_FILTER_LINEAR;

    uint4 clr = read_imageui( input, sampler, outCoord );

    float magnitude = clr.x;

    // Apply contrast and brightness (whitelevel, blacklevel). These must fit in 8-bit.
    magnitude -= blackLevel;
    magnitude /= ( ( whiteLevel - blackLevel ) / 255.0f );

    clr = (uint4)( magnitude, magnitude, magnitude, 1 );

    write_imageui( output, outCoord, clr );
}
