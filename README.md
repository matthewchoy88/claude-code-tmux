# claude-code-tmux

Simple tmux session manager for Claude Code.

## Install

```bash
./install.sh
```

Add to your shell config (`~/.zshrc`):
```bash
export PATH="$HOME/bin:$PATH"
fpath=($HOME/bin $fpath)
```

## Usage

```
cc new [name]       Create a new session (default: directory basename)
cc new -c [name]    Create a session with claude --continue
cc ls               List sessions
cc a [name]         Attach/switch to a session
cc kill [name]      Kill a session
cc rename old new   Rename a session
```

Session names support prefix matching: `cc a dat` matches `data-eng`.

Tab completion works for subcommands and session names.

## Tmux keys

| Key | Action |
|-----|--------|
| `ctrl-b s` | Session picker |
| `ctrl-b (` | Previous session |
| `ctrl-b )` | Next session |
| `ctrl-b d` | Detach |
