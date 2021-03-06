**TODO**
========
- [ ] Fix support for dynamic arrays (also `append` "intrinsic" function)
- [ ] Revisit Pointer, Reference semantics
- [ ] Do basic lifetime checks to avoid dangling pointers/invalid references
- [ ] Fix boolean expressions (mainly a grammar issue)
- [ ] Support implicit type conversions for structs via operator overloads
- [ ] Support binary, octal & hexadecimal numbers
- [ ] Consistent Name Mangling (currently ad-hoc) -> support `export` construct
- [ ] Distinguish pre/post increment/decrement operator functions (via mangled names?)
- [ ] Refactor/Redo types/type lists
- [x] Comments
- [ ] Expand the reach of Comments
- [ ] Support type construction from `type(<expression>)`
- [ ] Make "this" optionally implicit in struct functions (if variable name search fails in struct functions, check if they belong to "this")
- [ ] Template to generate syntax file for sublime text from grammar
- [ ] Proper structured bindings for pattern matching
- [ ] Guarantee copy elision
- [ ] Proper sublime_text tooling
- [ ] CodeGenError class (taking string error & state?)
- [ ] Support message passing channels & constructs
- [ ] Support atomic types
- [ ] Extensive testing
- [ ] Formalize the memory model in use
- [ ] Other random refactoring/fixes/improvements...
