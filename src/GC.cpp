#include "h/GC.h"
#include "h/VM.h"

std::vector<Object*> GC::m_objects{};

Compiler* GC::m_currentCompiler{nullptr};
VM* GC::m_currentVM{nullptr};

void GC::collectGarbage() {
    #ifdef DEBUG_LOG_GC
    std::cout << "-- GC BEGIN --\n";
    #endif

    markRoots();

    #ifdef DEBUG_LOG_GC
    std::cout << "-- GC END --\n";
    #endif
}

void GC::markRoots() {
    if (m_currentVM) {
        for (Value &value : m_currentVM->m_stack) {
            if (value.isObject()) {
                Object* object = value.asObject();
                object->mark();
                #ifdef DEBUG_LOG_GC
                std::cout << "Marked object \"" << *object << "\" at " << reinterpret_cast<void*>(object) << ".\n";
                #endif
            }
        }
    }
}

void GC::freeObject(Object* object) {
    #ifdef DEBUG_LOG_GC
    std::cout << "Freed object at " <<reinterpret_cast<void*>(object) << " of type " <<
    static_cast<int>(object->m_type) << ".\n";
    #endif

    delete object;
    m_objects.erase(std::find(m_objects.begin(), m_objects.end(), object));
}

void GC::freeObjects() {
    while (m_objects.begin() != m_objects.end()) {
        freeObject(*m_objects.begin());
    }
}

void GC::setCompiler(Compiler *compiler) {
    m_currentCompiler = compiler;
}

void GC::setVM(VM *vm) {
    m_currentVM = vm;
}
