cmake_minimum_required(VERSION 3.12)
project(enact)

set(CMAKE_CXX_STANDARD 14)

add_executable( enact
        src/Enact.cpp
        src/h/Enact.h
        src/h/common.h
        src/h/Token.h
        src/Scanner.cpp
        src/h/Scanner.h
        src/Chunk.cpp
        src/h/Chunk.h
        src/VM.cpp
        src/h/VM.h
        src/h/Parser.h
        src/Parser.cpp
        src/h/Value.h
        src/Value.cpp
        src/h/Object.h 
        src/Object.cpp
        src/ast/Expr.h
        src/ast/Stmt.h src/AstPrinter.cpp src/h/AstPrinter.h src/Analyser.cpp src/h/Analyser.h)
