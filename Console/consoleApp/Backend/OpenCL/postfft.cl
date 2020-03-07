__kernel void postfft_kernel(__global const float *input_re,
                              __global const float *input_imag,
                              __global float *prev_frame,
                              __write_only image2d_t output,
                              const unsigned int inputLength,
                              const float scaleFactor,
                              const unsigned int dcNoiseLevel,
                              const int doAverage,
                              const float oldFrameWeight_percent,
                              const float newFrameWeight_percent,
                              const int doInvert )
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int2 outCoord = (int2)(i, j);
    int offset = j * inputLength;
    float magnitude, re, im, tmp;

    re = input_re[i + offset];
    im = input_imag[i + offset];
    magnitude = re * re + im * im;
    magnitude = log10(magnitude) * scaleFactor - dcNoiseLevel;  // Scale to 8-bit, adjustable by caller
    tmp = magnitude;

    // Average with older frame
    if( doAverage )
    {
        magnitude = ( newFrameWeight_percent * magnitude ) + ( oldFrameWeight_percent * prev_frame[i + offset] );
    }
    prev_frame[ i + offset ] = tmp; // Store for later

    uint4 clr = 0;

    if( doInvert )
    {
        clr = (uint4)( 255 - magnitude, 255 - magnitude, 255 - magnitude, 1 );
    }
    else
    {
        clr = (uint4)( magnitude, magnitude, magnitude, 1 );
    }

    write_imageui( output, outCoord, clr );
}
