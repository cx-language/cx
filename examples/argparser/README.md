This directory contains a CLI argument parser library (`argparser.cx`,
`help.cx`) and a demo task manager (`main.cx`) built on it. The library
supports boolean flags, string and integer options, positional arguments,
git-style subcommands, and generated help text.

To build the demo, run `cx build`. This creates a `todo` executable.

```sh
./todo add "Buy almond milk"
./todo add --priority high "Fix bug"
./todo list
./todo done 2
./todo list --all
./todo --help
./todo add --help
```

Global options: `--db FILE` (`-d`, default `todo.db`), `--verbose` (`-v`).
