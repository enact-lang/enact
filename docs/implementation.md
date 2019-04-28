# Enact
## Planned interpreter structure

Mainly I'm just writing this down for myself, however potential contributors may find this interesting as well.
Enact is first and foremost an interpreted language, with the possibility of a JIT compiler in the future.

This repository contains source for a program that compiles Enact source code (`.en` files) down to a high level bytecode and runs 
this bytecode on a virtual machine.

Enact has a planned 5 pass interpreter:
- First, the source is parsed and converted into an AST. \[ done ✔️ \]
- Next, the AST is walked to resolve variables and check types. \[ done ✔️ \]
- Afterwards, the AST is walked again and compiled down to bytecode. \[ in progress 🚧 \]
- This bytecode is optimized by yet another pass. \[ not implemented ❌ \]
- Finally, the VM takes the bytecode and runs it. \[ in progress 🚧 \]

Currently, the focus of development is typechecking and semantic analysis in the AST.
