#include "h/GC.h"
#include "h/VM.h"
#include "h/Compiler.h"

size_t GC::m_bytesAllocated{0};
size_t GC::m_nextRun{1024 * 1024};

std::vector<Object*> GC::m_objects{};

Compiler* GC::m_currentCompiler{nullptr};
VM* GC::m_currentVM{nullptr};

std::vector<Object*> GC::m_greyStack{};

void GC::collectGarbage() {
    #ifdef DEBUG_LOG_GC
    std::cout << "-- GC BEGIN\n";
    size_t before = m_bytesAllocated;
    #endif

    markRoots();
    traceReferences();
    sweep();

    m_nextRun = m_bytesAllocated * GC_HEAP_GROW_FACTOR;

    #ifdef DEBUG_LOG_GC
    std::cout << "-- GC END: collected " << before - m_bytesAllocated << " bytes (from " << before << " to " <<
            m_bytesAllocated << "), next GC at " << m_nextRun << ".\n";
    #endif
}

void GC::markRoots() {
    if (m_currentCompiler) markCompilerRoots();
    if (m_currentVM) markVMRoots();
}

void GC::traceReferences() {
    while (!m_greyStack.empty()) {
        Object* object = m_greyStack.back();
        m_greyStack.pop_back();
        blackenObject(object);
    }
}

void GC::sweep() {
    for (auto it = m_objects.begin(); it != m_objects.end();) {
        Object* object = *it;
        if (object->isMarked()) {
            object->unmark();
            it++;
        } else {
            freeObject(object);
            it = m_objects.erase(it);
        }
    }
}

void GC::markCompilerRoots() {
    Compiler* compiler = m_currentCompiler;
    while (compiler != nullptr) {
        markObject(compiler->m_currentFunction);
        compiler = compiler->m_enclosing;
    }
}

void GC::markVMRoots() {
    for (Value &value : m_currentVM->m_stack) {
        markValue(value);
    }

    for (size_t i = 0; i < m_currentVM->m_frameCount; ++i) {
        markObject(m_currentVM->m_frames[i].closure);
    }

    for (UpvalueObject* upvalue = m_currentVM->m_openUpvalues; upvalue != nullptr; upvalue = upvalue->getNext()) {
        markObject(upvalue);
    }
}

void GC::markObject(Object *object) {
    if (!object || object->isMarked()) return;
    object->mark();

    m_greyStack.push_back(object);

    #ifdef DEBUG_LOG_GC
    std::cout << static_cast<void*>(object) << ": marked object [ " << *object << " ].\n";
    #endif
}

void GC::markValue(Value value) {
    if (value.isObject()) {
        markObject(value.asObject());
    }
}

void GC::markValues(const std::vector<Value>& values) {
    for (const Value& value : values) {
        markValue(value);
    }
}

void GC::blackenObject(Object *object) {
    #ifdef DEBUG_LOG_GC
    std::cout << static_cast<void*>(object) << ": blackened object [ " << *object << " ].\n";
    #endif

    switch (object->m_type) {
        case ObjectType::CLOSURE: {
            auto closure = object->as<ClosureObject>();
            markObject(closure->getFunction());
            for (UpvalueObject* upvalue : closure->getUpvalues()) {
                markObject(upvalue);
            }
            break;
        }

        case ObjectType::FUNCTION: {
            auto function = object->as<FunctionObject>();
            markValues(function->getChunk().getConstants());
            break;
        }

        case ObjectType::UPVALUE:
            markValue(object->as<UpvalueObject>()->getClosed());
            break;

        default:
            break;
    }
}

void GC::freeObject(Object* object) {
    #ifdef DEBUG_LOG_GC
    std::cout << static_cast<void*>(object) << ": freed object of type " <<
    static_cast<int>(object->m_type) << ".\n";
    #endif

    delete object;
}

void GC::freeObjects() {
    while (m_objects.begin() != m_objects.end()) {
        freeObject(*m_objects.begin());
        m_objects.erase(m_objects.begin());
    }
}

void GC::setCompiler(Compiler *compiler) {
    m_currentCompiler = compiler;
}

void GC::setVM(VM *vm) {
    m_currentVM = vm;
}
