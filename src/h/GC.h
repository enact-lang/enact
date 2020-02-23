#ifndef ENACT_GC_H
#define ENACT_GC_H

#include <vector>
#include "Object.h"

constexpr size_t GC_HEAP_GROW_FACTOR = 2;

class VM;
class Compiler;

class GC {
    static size_t m_bytesAllocated;
    static size_t m_nextRun;

    static std::vector<Object*> m_objects;

    static Compiler* m_currentCompiler;
    static VM* m_currentVM;

    static std::vector<Object*> m_greyStack;

    static void markRoots();
    static void traceReferences();
    static void sweep();
    static void markCompilerRoots();
    static void markVMRoots();
    static void markObject(Object* object);
    static void markValue(Value value);
    static void markValues(const std::vector<Value>& values);
    static void blackenObject(Object* object);
    
public:
    template <typename T, typename... Args>
    inline static T* allocateObject(Args&&... args) {
        static_assert(std::is_base_of_v<Object, T>,
                      "GC::allocateObject<T>: T must derive from Object.");

        m_bytesAllocated += sizeof(T);
        #ifdef DEBUG_STRESS_GC
        collectGarbage();
        #else
        if (m_bytesAllocated > m_nextRun) {
            collectGarbage();
        }
        #endif

        T* object = new T{args...};

        m_objects.push_back(object);

        #ifdef DEBUG_LOG_GC
        std::cout << static_cast<void*>(object) << ": allocated object of size " << sizeof(T) << " and type " <<
              static_cast<int>(static_cast<Object*>(object)->m_type) << ".\n";
        #endif

        return object;
    }

    static void collectGarbage();

    static void freeObject(Object* object);
    static void freeObjects();

    static void setCompiler(Compiler* compiler);
    static void setVM(VM* vm);
};

#endif //ENACT_GC_H
