# SafetyHook (vendored amalgamation)

Amalgamated build of [SafetyHook](https://github.com/cursey/safetyhook) v0.7.0
(`safetyhook-amalgamated-zydis.zip` release asset), including the bundled
[Zydis](https://github.com/zyantific/zydis) disassembler. Both are MIT-licensed.

Used for inline function detours on non-virtual engine functions (e.g.
`CCSPlayer_MovementServices::ProcessMovement` behind `Sdk::MovementHook`),
where SourceHook's vtable patching cannot reach. x86_64 Windows + Linux.

Do not edit these files; to upgrade, replace them with a newer release's
amalgamated assets and update the version here.
