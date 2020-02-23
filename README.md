<p align="center">
<img src="./docs/img/enact-logo-text.png" alt="Enact logo" width="60%" height="60%"></img>
</p>
<p align="center">
<a href="https://travis-ci.com/enact-lang/enact"><img src="https://travis-ci.com/enact-lang/enact.svg?branch=master" alt="Build Status" /></a>
<a href="https://www.codefactor.io/repository/github/enact-lang/enact"><img src="https://www.codefactor.io/repository/github/enact-lang/enact/badge" alt="CodeFactor" /></a><br>
</p>
<p align="center">
Enact is a new programming language that's not designed to be unique or groundbreaking. Instead, Enact takes already 
established technologies and paradigms and just makes them <strong>nicer</strong>.
</p>

## Example
```
// FizzBuzz in Enact

each i in 1..20:
    given (i % 3 == 0, i % 5 == 0):
        when (true, true):
            print("FizzBuzz")
        when (true, false):
            print("Fizz")
        when (false, true):
            print("Buzz")
        else:
            print(i)
    end
end
```

## Features
- A convenient optional static type system
- Composition-based user defined types
- Easy interop and modularity
- Clean and simple syntax

## Goals
- Not multi-paradigm, rather a blend of paradigms
- Familiar to both functional and imperative programmers
- Interop with C, C++ and Python
- Full ecosystem with tooling and packaging
