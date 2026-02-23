#!/usr/bin/env bash
# cc - Claude Code tmux session manager
set -euo pipefail

PREFIX="cc-"

usage() {
    cat <<EOF
cc - Claude Code session manager

Usage:
    cc new [name]       Create a new session (default name: directory basename)
    cc new -c [name]    Create a session with claude --continue
    cc ls               List sessions
    cc a [name]         Attach/switch to a session
    cc kill [name]      Kill a session
    cc rename old new   Rename a session
    cc help             Show this help

Sessions are prefixed with "cc-" to separate them from other tmux sessions.
EOF
}

session_name() {
    echo "${PREFIX}${1}"
}

# Strip prefix for display
display_name() {
    echo "${1#$PREFIX}"
}

# List cc session names (without prefix)
_cc_sessions() {
    tmux list-sessions -F '#{session_name}' 2>/dev/null | grep "^${PREFIX}" | sed "s/^${PREFIX}//" || true
}

# Find a session by prefix match. Returns full display name or empty.
match_session() {
    local input="$1"
    local exact matches

    # Exact match first
    if tmux has-session -t "$(session_name "$input")" 2>/dev/null; then
        echo "$input"
        return
    fi

    # Prefix match
    matches=$(_cc_sessions | grep "^${input}" || true)
    local count
    count=$(echo "$matches" | grep -c . || true)

    if [ "$count" -eq 1 ]; then
        echo "$matches"
    elif [ "$count" -gt 1 ]; then
        echo "Ambiguous match for '$input':" >&2
        echo "$matches" | sed 's/^/  /' >&2
        return 1
    else
        return 1
    fi
}

cmd_new() {
    local continue_flag=false
    if [ "${1:-}" = "-c" ]; then
        continue_flag=true
        shift
    fi

    local name="${1:-$(basename "$PWD")}"
    local session
    session=$(session_name "$name")

    if tmux has-session -t "$session" 2>/dev/null; then
        echo "Session '$name' already exists. Attaching..."
        cmd_attach "$name"
        return
    fi

    local cmd="claude"
    [ "$continue_flag" = true ] && cmd="claude --continue"

    tmux new-session -d -s "$session" -c "$PWD" "$cmd"

    # Attach or switch depending on whether we're already in tmux
    if [ -n "${TMUX:-}" ]; then
        tmux switch-client -t "$session"
    else
        tmux attach-session -t "$session"
    fi
}

cmd_ls() {
    local sessions
    sessions=$(tmux list-sessions -F '#{session_name}|#{session_created}|#{session_windows}|#{session_attached}' 2>/dev/null | grep "^${PREFIX}" || true)

    if [ -z "$sessions" ]; then
        echo "No Claude Code sessions."
        return
    fi

    printf "%-20s %-12s %s\n" "NAME" "WINDOWS" "STATUS"
    printf "%-20s %-12s %s\n" "----" "-------" "------"
    while IFS='|' read -r sname created windows attached; do
        local dname
        dname=$(display_name "$sname")
        local status="detached"
        [ "$attached" -gt 0 ] && status="attached"
        printf "%-20s %-12s %s\n" "$dname" "$windows" "$status"
    done <<< "$sessions"
}

cmd_attach() {
    local name="${1:-}"
    if [ -z "$name" ]; then
        echo "Usage: cc a <name>"
        echo ""
        cmd_ls
        return 1
    fi

    local matched
    matched=$(match_session "$name") || { echo ""; cmd_ls; return 1; }

    local session
    session=$(session_name "$matched")

    if [ -n "${TMUX:-}" ]; then
        tmux switch-client -t "$session"
    else
        tmux attach-session -t "$session"
    fi
}

cmd_kill() {
    local name="${1:-}"
    if [ -z "$name" ]; then
        echo "Usage: cc kill <name>"
        echo ""
        cmd_ls
        return 1
    fi

    local matched
    matched=$(match_session "$name") || return 1

    local session
    session=$(session_name "$matched")

    tmux kill-session -t "$session"
    echo "Killed session '$matched'."
}

cmd_rename() {
    local old="${1:-}"
    local new="${2:-}"
    if [ -z "$old" ] || [ -z "$new" ]; then
        echo "Usage: cc rename <old> <new>"
        return 1
    fi

    local old_session new_session
    old_session=$(session_name "$old")
    new_session=$(session_name "$new")

    if ! tmux has-session -t "$old_session" 2>/dev/null; then
        echo "Session '$old' not found."
        return 1
    fi

    tmux rename-session -t "$old_session" "$new_session"
    echo "Renamed '$old' -> '$new'."
}

# Main
case "${1:-}" in
    new)    shift; cmd_new "$@" ;;
    ls)     cmd_ls ;;
    a)      shift; cmd_attach "$@" ;;
    kill)   shift; cmd_kill "$@" ;;
    rename) shift; cmd_rename "$@" ;;
    help|--help|-h) usage ;;
    "")     usage ;;
    *)      echo "Unknown command: $1"; echo ""; usage; exit 1 ;;
esac
