%% This script demonstrates the usage of toolkitICL using Matlab to write the configuration file.

% Define an OpenCL kernel and write it to a file. Of course, you can
% write the kernel file in an editor for more complex use cases.
copy_kernel = [...
'#ifdef cl_khr_fp64\n' ...
'  #pragma OPENCL EXTENSION cl_khr_fp64 : enable\n' ...
'#else\n' ...
'  #error \"IEEE-754 double precision not supported by OpenCL implementation.\"\n' ...
'#endif\n' ...
'\n' ...
'kernel void copy(global COPYTYPE const* in, global COPYTYPE* out)\n' ...
'{\n' ...
'  const int gid = get_global_id(0);\n' ...
'  out[gid] = in[gid];\n' ...
'}\n'];
kernel_url = 'copy_kernel_m.cl';
io = fopen(kernel_url, 'w');
fprintf(io, copy_kernel);
fclose(io);

% input parameters
LENGTH = 32;

% generate input/output arrays for OpenCL
input = 0:(LENGTH-1);
output = zeros(size(input));

% write the configuration file
filename = 'copy_double_test_jl.h5';

% compiler arguments for OpenCL
hdf5write(filename, '/Kernel_Settings', '-DCOPYTYPE=double');

% URL of the file containing the kernels
hdf5write(filename, '/Kernel_URL', kernel_url, 'WriteMode', 'append');
% Instead, you could also write(io, "Kernel_Source", copy_kernel).

% kernels to be executed
hdf5write(filename, '/Kernels', {'copy'}, 'WriteMode', 'append');

% OpenCL ranges
h5create(filename, '/Global_Range', [1 3], 'Datatype', 'int32');
h5write(filename, '/Global_Range', [LENGTH,1,1]);

h5create(filename, '/Range_Start', [1 3], 'Datatype', 'int32');
h5write(filename, '/Range_Start', [0 0 0]);

h5create(filename, '/Local_Range', [1 3], 'Datatype', 'int32');
h5write(filename, '/Local_Range', [0,0,0]);

% data
h5create(filename, '/Data/in', [1 LENGTH], 'Datatype', 'double')
h5write(filename, '/Data/in', input)

h5create(filename, '/Data/out', [1 LENGTH], 'Datatype', 'double')
h5write(filename, '/Data/out', output)


% call toolkitICL
system(['toolkitICL -c ' filename]);


% check results
out_filename = ['out_' filename];
in_test = h5read(out_filename, '/Data/in');
if ~all(in_test == input)
    fprintf('Wrong result for "in"\n')
end

out_test = h5read(out_filename, '/Data/out');
if ~all(out_test == input)
    fprintf('Wrong result for "out"\n')
end

fprintf('Finished successfully.\n');

%% remove temporary files
if exist(filename, 'file')==2
  delete(filename);
end

if exist(kernel_url, 'file')==2
  delete(kernel_url);
end

if exist(out_filename, 'file')==2
  delete(out_filename);
end

