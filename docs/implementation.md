# Enact
## Planned interpreter structure

Mainly I'm just writing this down for myself, however potential contributors may find this interesting as well.
Enact is first and foremost an interpreted language, with the possibility of a JIT compiler in the future.

This repository contains source for a program that compiles Enact source code (`.en` files) down to a high level bytecode and runs 
this bytecode on a virtual machine.

Enact has a planned 5 pass interpreter:
- First, the source is parsed and converted into an AST. \[ done ✔️ \]
- Next, the AST is walked to resolve variables and check types. \[ in progress 🚧 \]
- Afterwards, the AST is walked again and compiled down to bytecode. \[ not implemented ❌ \]
- This bytecode is optimized by yet another pass. \[ not implemented ❌ \]
- Finally, the VM takes the bytecode and runs it. \[ not implemented ❌ \]

Currently, the focus of development is parsing the full syntax (as described by 
[the specification](https://github.com/enact-lang/spec)) down to an AST.
