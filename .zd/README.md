# zdev project memory

This directory holds durable planning state for `gecode`. Work normally
starts through the `zdev` harness skill. The skill uses `zd` for repository
discovery, validation, and other deterministic state changes.

Area directories live directly under `.zd/`. Names beginning with `_` are
reserved for local tool state.
