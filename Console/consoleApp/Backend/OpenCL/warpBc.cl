constant sampler_t NORM_SMPLR = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

__kernel void warp_kernel(__read_only image2d_t srcImg,
                          __write_only image2d_t dstImg,
                          __write_only image2d_t videoImg,
                          float catheterRadius_um,
                          float fInternalImagingMask_S,
                          float standardDepth_mm,
                          int standardDepth_S,
                          float rotationAngle_deg,
                          const int reverseDirection,
                          int width_px,
                          int height_px,
                          float fFractionOfCanvas,
                          int maxDepth_S,
			  int brightness,
			  int contrast )
{
    /*
     * Max depth in samples from FFT Output. Samples are not the same as Pixels since we don't display
     * 1 to 1 Samples to Pixels.
     */
    const float max_len_S        = maxDepth_S;
    const float fStandard_mm     = standardDepth_mm;
    const float fStandard_S      = standardDepth_S;
    const float fStandard_SPerMm = fStandard_S / fStandard_mm;

    /*
     * We work through the image at each *storage* location, choosing
     * the warped pixels to load based on the transform.
     */
    int2 storeCoord = (int2)( get_global_id( 0 ), get_global_id( 1 ) );
    float2 storeCoord_norm = convert_float2( storeCoord ) * (float2)( 1.0f / width_px, 1.0f / height_px );

    float2 loadCoord_norm;
    float2 videoLoadCoord_norm;

    // Move origin to center
    storeCoord_norm.x = storeCoord_norm.x - 0.5f;
    storeCoord_norm.y = storeCoord_norm.y - 0.5f;

    /*
     * Get angle and normalize to [0..1].  Theta is the position on the y-axis
     * of the source image; our source image is an unwrapped OCT image that is
     * some number of pixels deep by 360 degs.
     *
     * TBD: Use lookup table for speed? This call is worth about 5%-10% performance hit.
     */
    float theta = 0.0f;
    float directionMultiplier = 1.0f;

    if( reverseDirection )
    {
        // reverse the direction by making theta negative
        theta = -atan2pi( storeCoord_norm.y, storeCoord_norm.x ); // draw CCW
    }
    else
    {
        theta = atan2pi( storeCoord_norm.y, storeCoord_norm.x ); // draw CW
        directionMultiplier = -1.0f;
    }

    /*
     * r is the magnitude of the destination coordinate.
     * r is always positive and between 0 and 0.5.
     */
     float r = length( storeCoord_norm );

    /*
     * - r is the input variable in storeImage
     * - r_cath is normalized catheter size (normalized to storeImg)
     * - slope is rise over run
     * - y_val is going to be the internal mask (normalized to loadImg)
     */
    float y_val_loadNorm = (float)fInternalImagingMask_S * 1.0f / 1024.0f;
    float c3_px; // See C3 from SDS.
    float r_max = fFractionOfCanvas;
    float r_cath_px;

    float scaleFactor = ( 1024.0f - y_val_loadNorm ) / max_len_S;

    c3_px = max_len_S;
    // pass in image depth 3.23
    r_cath_px = catheterRadius_um * fStandard_SPerMm / 1000.0f; // 1000.0f um per mm

    float c3_loadNorm = (float)c3_px * 1.0f / 1024.0f;

    float r_cath_storeNorm = r_cath_px / ( r_cath_px + c3_px ) * r_max;

    // slope = rise/run
    float rise  = c3_loadNorm;
    float run   = r_max - r_cath_storeNorm;
    float slope = rise / run;

    // The meat of warp.cl, this is the loadImg pixel lookup.
    loadCoord_norm.x = y_val_loadNorm + slope * ( r - r_cath_storeNorm );

    videoLoadCoord_norm.x = loadCoord_norm.x;

    // Shift and divide to match range of atan and output range
    loadCoord_norm.y = ( theta + 1.0f ) / 2.0f;

    /*
     * Find the rotated point relative to the computer loadCoord_norm location.
     * Since the coordinates are normalized, we just have to normalize the
     * rotation to the range to [0..1].  We clamp videoLoadCoord_norm to [0..1].
     */
    videoLoadCoord_norm.y = fmod( ( loadCoord_norm.y + directionMultiplier * ( rotationAngle_deg / (float)360.0f ) ), 1.0f );

    if( videoLoadCoord_norm.y < 0.0f )
    {
        videoLoadCoord_norm.y = videoLoadCoord_norm.y + 1.0f;
    }

    uint4 pixel;
    int   ipixel;
    float fpixel;
    float fcontrast;
    float contrastCorrection;

    /*
     * Draw black if:
     *   corners of the canvas,
     *   looking up outside the load image,
     *   looking up inside the internalImagingMask.
     */
    if( ( r > r_max ) || ( loadCoord_norm.x > 1.0f ) || ( loadCoord_norm.x < y_val_loadNorm ) )
    {
        pixel = (uint4){ 0, 0, 0, 1 }; // give a black center circle to the sector
    }
    else // everything else inside should be the pixel from the load image
    {
        pixel = read_imageui( srcImg, NORM_SMPLR, videoLoadCoord_norm );
        ipixel = pixel.s0;
	ipixel = ipixel + brightness;
	if(ipixel < 0) ipixel = 0;
	if(ipixel > 255) ipixel = 255;
	pixel.s0 = ipixel;

	fcontrast = contrast;
        contrastCorrection = (259.0f*(fcontrast+255.0f))/(255.0f*(259.0f-fcontrast));
	fpixel = pixel.s0;
        fpixel = contrastCorrection*(fpixel-128.0f) + 128.0f;
	ipixel = fpixel;
        if( ipixel < 0 ) ipixel = 0;
        if( ipixel > 255 ) ipixel = 255;
	pixel.s0 = ipixel;
    }

    // Write to the image for display and for the video
    write_imageui(   dstImg, storeCoord, pixel );
}
