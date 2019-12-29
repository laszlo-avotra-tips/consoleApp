% generateLogTableFile.m
%
% Create a file with precomputed logarithm table
% for comparison with the output of dsp.cpp
%
% Author: Chris White
logArray = (10 * log10([0:65535])) * (65535 / (10 * log10(65535)));
fid = fopen('logtable.txt','w');
fprintf(fid, '%6.0f', floor(logArray));
fclose(fid);