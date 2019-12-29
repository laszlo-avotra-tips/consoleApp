__kernel void rescale_kernel(__global const unsigned short *input,
                             __global float *output,
                             __global const float *fractionalSamples,
                             __global const float *wholeSamples,
                             __global const float *window,
                             const unsigned int inputLength,
                             const unsigned int outputLength)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    float interpSample;
    int write_offset = j * outputLength;
    int read_offset = j * inputLength;
    int input_pos;
    unsigned int sampleIndex;
    sampleIndex = (int)wholeSamples[i];
       
    if ((sampleIndex + 1) > inputLength) {
       return;
    }

    input_pos = read_offset + sampleIndex;
    interpSample = (float)(input[input_pos + 1] - input[input_pos]) *
                       fractionalSamples[i];
    interpSample = interpSample + input[input_pos];

    // Apply the passed in window function and set the output
    output[i + write_offset] = interpSample * window[i];
}

