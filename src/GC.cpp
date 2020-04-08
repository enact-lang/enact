#include "h/Context.h"
#include "h/GC.h"

GC::GC(Context& context) : m_context{context} {
}

GC::~GC() {
    freeObjects();
}


Object* GC::cloneObject(Object *object) {
    m_bytesAllocated += object->size();
    if (m_bytesAllocated > m_nextRun || m_context.options.flagEnabled(Flag::DEBUG_STRESS_GC)) {
        collectGarbage();
    }

    Object* cloned = object->clone();
    m_objects.push_back(cloned);

    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << static_cast<void *>(cloned) << ": allocated object of size " << cloned->size() << " and type " <<
                  static_cast<int>(static_cast<Object*>(cloned)->m_type) << ".\n";
    }

    return cloned;
}

void GC::collectGarbage() {
    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << "-- GC BEGIN\n";
    }

    size_t before = m_bytesAllocated;

    markRoots();
    traceReferences();
    sweep();

    m_nextRun = m_bytesAllocated * GC_HEAP_GROW_FACTOR;

    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << "-- GC END: collected " << before - m_bytesAllocated << " bytes (from " << before << " to " <<
                  m_bytesAllocated << "), next GC at " << m_nextRun << ".\n";
    }
}

void GC::markRoots() {
    markCompilerRoots();
    markVMRoots();
}

void GC::markCompilerRoots() {
    Compiler* compiler = &m_context.currentCompiler();
    while (compiler != nullptr) {
        if (compiler->m_currentFunction) markObject(compiler->m_currentFunction);
        compiler = compiler->m_enclosing;
    }
}

void GC::markVMRoots() {
    for (Value &value : m_context.vm.m_stack) {
        markValue(value);
    }

    for (size_t i = 0; i < m_context.vm.m_frameCount; ++i) {
        markObject(m_context.vm.m_frames[i].closure);
    }

    for (UpvalueObject* upvalue = m_context.vm.m_openUpvalues; upvalue != nullptr; upvalue = upvalue->getNext()) {
        markObject(upvalue);
    }
}

void GC::markObject(Object *object) {
    if (!object || object->isMarked()) return;
    object->mark();

    m_greyStack.push_back(object);

    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << static_cast<void *>(object) << ": marked object [ " << *object << " ].\n";
    }
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

void GC::traceReferences() {
    while (!m_greyStack.empty()) {
        Object* object = m_greyStack.back();
        m_greyStack.pop_back();
        blackenObject(object);
    }
}

void GC::blackenObject(Object *object) {
    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << static_cast<void *>(object) << ": blackened object [ " << *object << " ].\n";
    }

    switch (object->m_type) {
        case ObjectType::INSTANCE: {
            auto* instance = object->as<InstanceObject>();
            markObject(instance->getStruct());
            for (Value property : instance->properties()) {
                markValue(property);
            }
            break;
        }

        case ObjectType::STRUCT: {
            auto* struct_ = object->as<StructObject>();
            for (ClosureObject* method : struct_->methods()) {
                markObject(method);
            }
            for (Value assoc : struct_->assocs()) {
                markValue(assoc);
            }
            break;
        }

        case ObjectType::CLOSURE: {
            auto* closure = object->as<ClosureObject>();
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

void GC::freeObject(Object* object) {
    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << static_cast<void *>(object) << ": freed object of type " <<
                  static_cast<int>(object->m_type) << ".\n";
    }

    delete object;
}

void GC::freeObjects() {
    while (m_objects.begin() != m_objects.end()) {
        freeObject(*m_objects.begin());
        m_objects.erase(m_objects.begin());
    }
}