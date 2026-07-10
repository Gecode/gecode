% Blackbox annotations execute user-provided code. Use them only for trusted
% models and trusted executable/library paths.
%
% blackbox_dll loads a native library and calls:
%
%   void fzn_blackbox(
%     const int64_t* int_in, size_t n_int_in,
%     const double* float_in, size_t n_float_in,
%     int64_t* int_out, size_t n_int_out,
%     double* float_out, size_t n_float_out)
%
% The library may also export:
%
%   void fzn_initialize(const char** args, size_t n_args)
%
% On Windows these functions use the Gecode blackbox calling convention. Native
% library blackboxes can be called concurrently by parallel search workers and
% must be thread-safe.
%
% blackbox_exec starts a persistent subprocess. For each call, Gecode writes one
% line to stdin:
%
%   comma-separated integer inputs ; comma-separated float inputs
%
% The process must answer with one line in the same format for the expected
% integer and float outputs. Outputs must be finite and in Gecode's numeric
% ranges. Response lines are limited to 1 MiB. The helper is trusted code and
% must keep reading requests and writing complete newline-terminated responses;
% otherwise it can block the solver. During parallel search each worker thread
% gets its own process.
%
% All blackboxes must be deterministic from FlatZinc's point of view: the same
% input arrays must produce the same output arrays. Implementations may cache
% internally as long as the observable result does not depend on call order.
annotation blackbox_dll(string: library);
annotation blackbox_dll(string: library, array[int] of string: args);

annotation blackbox_exec(string: command);
annotation blackbox_exec(string: command, array[int] of string: args);
