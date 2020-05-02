constant sampler_t NORM_SMPLR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void line_avg_kernel( __read_only image2d_t srcImg,
                               __write_only image2d_t dstImg,
                               const unsigned int NumLinesToAvg )
{
    float SCALE_x = 1.0f;
    float SCALE_y = 1.0f / (float)NumLinesToAvg;

int2 srcImgDim = get_image_dim( srcImg );
int2 dstImgDim = get_image_dim( dstImg );

    if( NumLinesToAvg > 1.0f )
    {
        float2 input_coord = (float2)( ((get_global_id(0) + 0.4995f) / SCALE_x),
                                       ((get_global_id(1) + 0.4995f) / SCALE_y) );

        int2 output_coord = (int2)( get_global_id( 0 ), get_global_id( 1 ) );
        if( output_coord.x >= dstImgDim.x || output_coord.y >= dstImgDim.y )  // don't index out of range
        {
            return;
        }


        float4 pixel;
        int2 ipos = convert_int2(input_coord);
        if( ipos.x < 0 || ipos.x >= srcImgDim.x ||
            ipos.y < 0 || ipos.y >= srcImgDim.y )
        {
            pixel = 0;
        }
        else
        {
            pixel = read_imagef( srcImg, NORM_SMPLR, input_coord );
        }

        // Write to the image for display and for the video
        write_imagef( dstImg, output_coord, pixel );
    }
}
