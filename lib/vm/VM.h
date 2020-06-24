#ifndef ENACT_VM_H
#define ENACT_VM_H

#include <array>
#include <optional>

#include "../bytecode/Chunk.h"
#include "../common.h"
#include "../value/Object.h"
#include "../value/Value.h"

namespace enact {
    class CompileContext;

    enum class CompileResult;

    constexpr size_t FRAMES_MAX = 64;

    struct CallFrame {
        ClosureObject *closure;
        const uint8_t *ip;
        size_t slotsBegin;
    };

    class VM {
        friend class GC;

        CompileContext &m_context;

        std::vector<Value> m_stack{};

        std::array<CallFrame, FRAMES_MAX> m_frames{CallFrame{nullptr, nullptr, 0}};
        size_t m_frameCount = 0;
        CallFrame *m_frame = nullptr;

        UpvalueObject *m_openUpvalues = nullptr;

        size_t m_pc = 0;

        void executionLoop(FunctionObject *function);

    public:
        explicit VM(CompileContext &context);

        CompileResult run(FunctionObject *function);

        inline void callFunction(ClosureObject *closure, uint8_t argCount);

        inline void callConstructor(StructObject *struct_, uint8_t argCount);

        inline void callNative(NativeObject *native, uint8_t argCount);

        inline void checkFunctionCallable(const FunctionType *type, uint8_t argCount);

        inline void checkConstructorCallable(const ConstructorType *type, uint8_t argCount);

        inline void encloseFunction(FunctionObject *function);

        inline void makeConstructor(std::shared_ptr<const ConstructorType> type);

        inline uint8_t readByte();

        inline uint16_t readShort();

        inline uint32_t readLong();

        inline Value readConstant();

        inline Value readConstantLong();

        void push(Value value);

        Value pop();

        Value peek(size_t depth);

        UpvalueObject *captureUpvalue(uint32_t location);

        void closeUpvalues(uint32_t last);

    private:
        class RuntimeError : public std::runtime_error {
        public:
            RuntimeError() : std::runtime_error{
                    "Fatal (internal):\n    VM::RuntimeError was left uncaught.\nThis is a COMPILER BUG, please report it at https://github.com/enact-lang/enact."} {}
        };

        RuntimeError runtimeError(const std::string &msg);
    };
}

#endif //ENACT_VM_H
