# Delta Compiler Manual

## Commands

### Package Commands

Commands available in a package source tree:
- `delta build` - compile and link the package
- `delta check` - type-check the package without building it (not implemented yet)
- `delta test` - build the package if needed and then run its tests (not implemented yet)
- `delta run` - build the package if needed and then run it

### Non-Package Commands

Commands available everywhere:
- `delta` - enter the REPL
- `delta <files>` - compile the given files into an executable
- `delta run <files>` - compile the given files into an executable and run it
- `delta init` - initialize a new package interactively (not implemented yet)
- `delta init --exe` - initialize a new executable package (not implemented yet)
- `delta init --lib` - initialize a new library package (not implemented yet)

### Additional Options

Run `delta help` for a list of command-line flags that can be passed to the
above commands.
