% generateTransformTable.m
%
% Generate a table of comparison values for the dsp
% test transform portion.
%
% Author: Chris White

 fid = fopen('transformtable.txt','w');

 figure(1);
 
for freq=0:2047
    freqTable   = (sin((1:2048) * freq / 2048) + 1.0) * 8192;
    fprintf(fid, '%6.0f', freqTable);
    fprintf(fid, '\n');
    FFTArray    = fft(freqTable/32.*hann(2048)', 2048);
    FFTArray    = int16(sqrt(FFTArray.*conj(FFTArray)));
    FFTArray(1) = 0.0;
    result      = abs(10*log10(double(FFTArray(1:512))+1))*(65535 / (10 * log10(65535)));
    fprintf(fid, '%6.0f', floor(result));
    fprintf(fid, '\n');
end

fclose(fid);