<p align="center">
<img src="./docs/img/enact-logo-text.png" alt="Enact logo" width="60%" height="60%"></img>
</p>
<p align="center">
<a href="https://travis-ci.com/enact-lang/enact"><img src="https://travis-ci.com/enact-lang/enact.svg?branch=master" alt="Build Status" /></a>
<a href="https://www.codefactor.io/repository/github/enact-lang/enact"><img src="https://www.codefactor.io/repository/github/enact-lang/enact/badge" alt="CodeFactor" /></a><br>
</p>
<p align="center">
Enact is a new compiled general-purpose programming language that's not designed to be unique or groundbreaking. 
Instead, Enact aims to be familiar, taking advantage of already established techniques and paradigms and making them
<strong>nicer</strong>.
</p>

## Example
```
// FizzBuzz in Enact

func fizzBuzz(n int) {
    for i in 1...n {
        switch (i % 3 == 0, i % 5 == 0) {
            case (true, false) => print("Fizz");
            case (false, true) => print("Buzz");
            case (true, true)  => print("FizzBuzz");
            default            => print(n);
        }
    }
}
```

## Features
- Static types that help, not hinder
- Efficient compile-time memory management
- Easy-to-use generics, hygienic macros
- Pattern matching and tail-calls
- Clean and familiar syntax
- Built-in build system and package management

## Goals
- Easy to pick up from other compiled languages like C, C++, Rust and Go
- More memory-safe than C, more approachable than Rust, more peformant than Go
- Small standard library with a strong ecosystem of external packages