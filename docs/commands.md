# Delta Commands

## Package Commands

Commands available in a package source tree:
- `delta build` - compile and link the package
- `delta check` - type-check the package without building it
- `delta test` - build the package if needed and then run its tests
- `delta run` - build the package if needed and then run it

## Non-Package Commands

Commands available everywhere:
- `delta` - enter the REPL
- `delta <files>` - compile the given files into an executable
- `delta run <files>` - compile the given files into an executable and run it
- `delta init` - initialize a new package interactively
- `delta init -exe` - initialize a new executable package
- `delta init -lib` - initialize a new library package
