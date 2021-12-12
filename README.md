# long-tracer

The Linux `perf` command allows for process tracing using intel\_pt, and is ideal for most use-cases.
`perf` offers a linear buffer method that works for shorter traces, but intel\_pt will log data faster than it can be written to disk.
`perf` also offers a ring buffered method that is ideal for capturing shorter traces targeting a specific event.

This project attempts to use intel\_pt to perform a complete trace of a target, at the expense of performance.
This is accomplished, by using a large linear buffer, and stopping the target every time data is made available until that data has been written to disk.

This trace data can later be analyzed using [libipt](https://github.com/intel/libipt).

**WARNING: this tracer can dump gigabytes of data very rapidly**

## Usage

`./tracer <target_program>`

The aux data will be written to `./aux.data`.

## Reviewing data with libipt

Aux data can be anaylzed using libipt's `ptdump` and `ptxed`.

## Example

A sample target performing a long-running fib calculation is located in `fib_forever_ex`.

`./tracer ./fib_forever_ex/a.out`

`ptdump ./aux.data`
