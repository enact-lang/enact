#ifndef ENACT_GC_H
#define ENACT_GC_H

#include <vector>
#include "Object.h"

class VM;
class Compiler;

class GC {
    static std::vector<Object*> m_objects;

    static Compiler* m_currentCompiler;
    static VM* m_currentVM;

public:
    template <typename T, typename... Args>
    inline static T* allocateObject(Args&&... args) {
        static_assert(std::is_base_of_v<Object, T>,
                      "GC::allocateObject<T>: T must derive from Object.");

        T* object = new T{args...};

        #ifdef DEBUG_STRESS_GC
        collectGarbage();
        #endif

        m_objects.push_back(object);

        #ifdef DEBUG_LOG_GC
        std::cout << "Allocated object at " << reinterpret_cast<void*>(object) << " of size " << sizeof(T) << " and of type " <<
              static_cast<int>(reinterpret_cast<Object*>(object)->m_type) << ".\n";
        #endif

        return object;
    }

    static void collectGarbage();
    static void markRoots();

    static void freeObject(Object* object);
    static void freeObjects();

    static void setCompiler(Compiler* compiler);
    static void setVM(VM* vm);
};

#endif //ENACT_GC_H
