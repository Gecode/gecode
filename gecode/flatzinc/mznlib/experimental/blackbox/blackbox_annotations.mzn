% Blackbox annotations execute user-provided code. Use them only for trusted
% models and trusted executable/library paths.
%
% blackbox_dll loads a native library with the following C ABI:
%
%   extern "C" {
%   void* fzn_init(const char** args, size_t n_args);
%   void* fzn_clone(void* instance);
%   void fzn_blackbox(
%     void* instance,
%     const int64_t* int_in, size_t n_int_in,
%     const double* float_in, size_t n_float_in,
%     int64_t* int_out, size_t n_int_out,
%     double* float_out, size_t n_float_out);
%   void fzn_free(void* instance);
%   }
%
% fzn_blackbox is required. fzn_init, fzn_clone, and fzn_free are optional;
% exporting fzn_init requires exporting fzn_clone. If fzn_init is absent, the
% library is stateless: fzn_blackbox receives NULL and can be called
% concurrently. If fzn_free is absent, Gecode does not release library state.
%
% Gecode calls fzn_init once for each blackbox_dll constraint, with the
% annotation arguments. In threaded builds its result is a prototype: each
% calling thread receives and reuses one fzn_clone result, and calls using the
% same clone are serialized. Different clones can be called concurrently. In
% builds without threads, fzn_blackbox receives the fzn_init result directly.
% When fzn_free is exported, Gecode calls it once for every fzn_init and
% fzn_clone result: once for the root and once for each clone.
%
% On Windows, export all functions with __declspec(dllexport) and use
% __stdcall. On other systems, use the ordinary C calling convention.
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
% All blackboxes must be deterministic from FlatZinc's point of view: equal
% input arrays must produce equal output arrays, regardless of call order or
% internal instance-state evolution.
annotation blackbox_dll(string: library);
annotation blackbox_dll(string: library, array[int] of string: args);

annotation blackbox_exec(string: command);
annotation blackbox_exec(string: command, array[int] of string: args);
