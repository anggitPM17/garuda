#pragma once
// Garuda Virtual Machine (GVM) — Real BEAM Opcode & Register Interpreter
// Mengimplementasikan Register X/Y, Process Stack, Call Stack Frame Isolation, Instruction Pointer (IP),
// Reductions Counter, dan Dispatcher Opcode berbasis Virtual Machine BEAM sejati.
#include "../core/runtime.hpp"
#include "../core/term.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace garuda::beam {

enum class Opcode : uint8_t {
    OpMove = 1,          // move src, dst
    OpGetTupleElem = 2,  // get_tuple_element tuple, idx, dst
    OpMakeTuple = 3,     // make_tuple dst, size
    OpCall = 4,          // call label
    OpCallExt = 5,       // call_ext BIF
    OpSend = 6,          // send x[0]=to, x[1]=msg
    OpLoopRec = 7,       // loop_rec label
    OpWait = 8,          // wait label
    OpReturn = 9,        // return / ret
    OpAllocate = 10,     // allocate stack_size
    OpDeallocate = 11,   // deallocate
    OpJump = 12,         // jump label
    OpIsAtom = 13,       // is_atom src, fail_label
    OpIsInteger = 14,    // is_integer src, fail_label
    OpAdd = 15,          // add x[a], x[b], x[c]
    OpSub = 16,          // sub x[a], x[b], x[c]
    OpMul = 17,          // mul x[a], x[b], x[c]
    OpDiv = 18,          // div x[a], x[b], x[c]
    OpEq = 19,           // eq x[a], x[b], x[c]
    OpNeq = 20,          // neq x[a], x[b], x[c]
    OpLt = 21,           // lt x[a], x[b], x[c]
    OpGt = 22,           // gt x[a], x[b], x[c]
    OpLte = 23,          // lte x[a], x[b], x[c]
    OpGte = 24,          // gte x[a], x[b], x[c]
    OpJumpIfFalse = 25,  // jump_if_false cond_reg, target_ip
    OpPrint = 26,        // print reg
    OpHalt = 27,         // halt
    OpSpawn = 28,        // spawn fun_ref -> pid
    OpSendMsg = 29,      // send(X[target], X[msg])
    OpSelf = 30,         // X[target] = self()
    OpReceive = 31,      // receive pop -> X[0]
    OpMatchTuple = 32,   // match tuple
    OpMatchLiteral = 33, // match literal
    OpBind = 34,         // bind pattern var
    OpTailCall = 35,     // tail call -> direct jump without pushing call stack
    OpConcat = 36,       // string / list concatenation (++)
    OpTryBegin = 37,     // try_begin catch_label
    OpTryEnd = 38,       // try_end
    OpThrow = 39,        // throw x[reason_reg]
    OpGetListTail = 42,  // get_list_tail src_reg, dst_reg
    OpGetListLen = 43,   // get_list_len src_reg, dst_reg
    OpGetListElem = 44,  // get_list_elem src_reg, index_lit, dst_reg
    OpMakeList = 45      // make_list dst, item0, item1, ...
};

enum class RegType { X, Y, Literal };

struct Operand {
    RegType type = RegType::Literal;
    std::size_t index = 0;
    Term literal;
};

struct Instruction {
    Opcode op;
    std::vector<Operand> args;
    std::size_t jump_label = 0;
    std::uint32_t line_info = 0;
};

struct GvmProcessState {
    Pid self_pid{1};                                 // Process Self PID
    std::array<Term, 256> x_regs;                    // Register X (256 registers - BEAM Standard)
    std::vector<Term> y_stack;                       // Register Y (Process Stack)
    std::vector<std::size_t> call_stack;             // Return IP Call Stack
    std::vector<std::array<Term, 256>> frame_stack;  // Frame Isolation Stack
    std::vector<std::size_t> try_stack;              // Exception Handler Stack
    std::size_t ip = 0;                              // Instruction Pointer
    std::size_t tail_call_target = 0;                // Target IP for Trampolined TailCall
    std::uint32_t reductions = 2000;                 // Reduction counter per slice
    bool halted = false;
    Term return_value = atom("ok");
};

using SpawnIpCallback = std::function<Pid(std::size_t target_ip)>;
using SendMsgCallback = std::function<void(Pid target, Term msg)>;
using ReceiveMsgCallback = std::function<std::optional<Term>(Pid self)>;

class GvmInterpreter {
public:
    explicit GvmInterpreter(std::vector<Instruction> code)
        : code_(std::move(code)) {}

    void set_callbacks(SpawnIpCallback spawn_cb, SendMsgCallback send_cb, ReceiveMsgCallback recv_cb) {
        spawn_cb_ = std::move(spawn_cb);
        send_cb_ = std::move(send_cb);
        recv_cb_ = std::move(recv_cb);
    }

    // Eksekusi Virtual Machine dengan preemption reduction counter (2000 reductions per slice)
    Status execute_slice(Process& proc, GvmProcessState& vm) {
        vm.reductions = 2000;

        while (vm.ip < code_.size() && vm.reductions > 0 && !vm.halted) {
            const auto& inst = code_[vm.ip];
            vm.reductions--;

            switch (inst.op) {
                case Opcode::OpMove: {
                    Term val = read_operand(vm, inst.args[0]);
                    write_operand(vm, inst.args[1], val);
                    vm.ip++;
                    break;
                }
                case Opcode::OpGetTupleElem: {
                    Term tuple_term = read_operand(vm, inst.args[0]);
                    std::size_t idx = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[1])));
                    const Term* elem = tuple_at(tuple_term, idx);
                    if (elem) {
                        write_operand(vm, inst.args[2], *elem);
                    }
                    vm.ip++;
                    break;
                }
                case Opcode::OpMakeTuple: {
                    std::vector<Term> items;
                    for (std::size_t i = 1; i < inst.args.size(); ++i) {
                        items.push_back(read_operand(vm, inst.args[i]));
                    }
                    write_operand(vm, inst.args[0], tuple_of(items));
                    vm.ip++;
                    break;
                }
                case Opcode::OpSend: {
                    Term to = vm.x_regs[0];
                    Term msg = vm.x_regs[1];
                    if (auto* pid = to.get_if<Pid>()) {
                        proc.send(*pid, msg);
                    }
                    vm.ip++;
                    break;
                }
                case Opcode::OpSendMsg: {
                    Term target_term = read_operand(vm, inst.args[0]);
                    Term msg_term = read_operand(vm, inst.args[1]);
                    if (auto* target_pid = target_term.get_if<Pid>()) {
                        if (send_cb_) send_cb_(*target_pid, msg_term);
                    }
                    vm.ip++;
                    break;
                }
                case Opcode::OpSpawn: {
                    std::size_t start_ip = 0;
                    if (auto* ip_val = inst.args[0].literal.get_if<std::int64_t>()) {
                        start_ip = static_cast<std::size_t>(*ip_val);
                    }
                    if (spawn_cb_) {
                        Pid child_pid = spawn_cb_(start_ip);
                        write_operand(vm, inst.args[1], Term(child_pid));
                    }
                    vm.ip++; // BUMP IP!
                    break;
                }
                case Opcode::OpReceive: {
                    if (recv_cb_) {
                        auto msg = recv_cb_(vm.self_pid);
                        if (msg.has_value()) {
                            std::size_t reg_idx = inst.args.empty() ? 0 : inst.args[0].index;
                            vm.x_regs[reg_idx] = *msg;
                            vm.ip++; // BUMP IP!
                            break;
                        } else {
                            // Mailbox empty -> Yield process to scheduler without advancing IP
                            return Status::Wait;
                        }
                    }
                    vm.ip++;
                    break;
                }
                case Opcode::OpSelf: {
                    write_operand(vm, inst.args[0], Term(vm.self_pid));
                    vm.ip++;
                    break;
                }
                case Opcode::OpLoopRec: {
                    auto msg = proc.receive();
                    if (msg) {
                        vm.x_regs[0] = msg->from;
                        vm.x_regs[1] = msg->body;
                        vm.ip++;
                    } else {
                        return Status::Wait; // Yield process to scheduler
                    }
                    break;
                }
                case Opcode::OpAdd: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 + v2));
                    vm.ip++;
                    break;
                }
                case Opcode::OpSub: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 - v2));
                    vm.ip++;
                    break;
                }
                case Opcode::OpMul: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 * v2));
                    vm.ip++;
                    break;
                }
                case Opcode::OpDiv: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v2 != 0 ? v1 / v2 : 0));
                    vm.ip++;
                    break;
                }
                case Opcode::OpEq: {
                    Term v1 = read_operand(vm, inst.args[0]);
                    Term v2 = read_operand(vm, inst.args[1]);
                    bool eq = (v1 == v2);
                    write_operand(vm, inst.args[2], Term(eq ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpNeq: {
                    Term v1 = read_operand(vm, inst.args[0]);
                    Term v2 = read_operand(vm, inst.args[1]);
                    bool neq = (v1 != v2);
                    write_operand(vm, inst.args[2], Term(neq ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpLt: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 < v2 ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpGt: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 > v2 ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpLte: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 <= v2 ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpGte: {
                    std::int64_t v1 = get_int_val(read_operand(vm, inst.args[0]));
                    std::int64_t v2 = get_int_val(read_operand(vm, inst.args[1]));
                    write_operand(vm, inst.args[2], Term(v1 >= v2 ? 1LL : 0LL));
                    vm.ip++;
                    break;
                }
                case Opcode::OpConcat: {
                    std::string v1 = to_string(read_operand(vm, inst.args[0]));
                    std::string v2 = to_string(read_operand(vm, inst.args[1]));
                    if (v1.length() >= 2 && v1.front() == '"' && v1.back() == '"') v1 = v1.substr(1, v1.length() - 2);
                    if (v2.length() >= 2 && v2.front() == '"' && v2.back() == '"') v2 = v2.substr(1, v2.length() - 2);
                    write_operand(vm, inst.args[2], Term(v1 + v2));
                    vm.ip++;
                    break;
                }
                case Opcode::OpJump: {
                    std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[0])));
                    vm.ip = target_ip;
                    break;
                }
                case Opcode::OpJumpIfFalse: {
                    Term cond_val = read_operand(vm, inst.args[0]);
                    std::int64_t cond_int = get_int_val(cond_val);
                    if (cond_int == 0) {
                        std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[1])));
                        vm.ip = target_ip;
                    } else {
                        vm.ip++;
                    }
                    break;
                }
                case Opcode::OpTailCall: {
                    std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[0])));
                    vm.ip = target_ip;
                    break;
                }
                case Opcode::OpCall: {
                    std::size_t target_ip = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[0])));
                    vm.call_stack.push_back(vm.ip + 1);
                    vm.frame_stack.push_back(vm.x_regs); // SAVE PARENT CALLER FRAME!
                    vm.ip = target_ip;
                    break;
                }
                case Opcode::OpReturn: {
                    Term ret_val = vm.x_regs[0]; // Capture return value
                    if (!vm.call_stack.empty()) {
                        vm.ip = vm.call_stack.back();
                        vm.call_stack.pop_back();
                        if (!vm.frame_stack.empty()) {
                            vm.x_regs = vm.frame_stack.back(); // RESTORE PARENT CALLER FRAME!
                            vm.frame_stack.pop_back();
                        }
                        vm.x_regs[0] = ret_val; // Write return value to X[0]
                    } else {
                        vm.halted = true;
                        return Status::Done;
                    }
                    break;
                }
                case Opcode::OpPrint: {
                    Term val = read_operand(vm, inst.args[0]);
                    std::cout << "🖨️ [GARUDA PRINT] " << to_string(val) << std::endl;
                    vm.ip++;
                    break;
                }
                case Opcode::OpAllocate: {
                    std::size_t words = static_cast<std::size_t>(get_int_val(read_operand(vm, inst.args[0])));
                    vm.y_stack.resize(vm.y_stack.size() + words, Term());
                    vm.ip++;
                    break;
                }
                case Opcode::OpDeallocate: {
                    vm.y_stack.clear();
                    vm.ip++;
                    break;
                }
                case Opcode::OpHalt: {
                    vm.halted = true;
                    return Status::Done;
                }
            case Opcode::OpTryBegin: {
                if (inst.args.size() == 1 && inst.args[0].type == RegType::Literal) {
                    std::size_t catch_ip = static_cast<std::size_t>(inst.args[0].literal.get_if<std::int64_t>() ? *inst.args[0].literal.get_if<std::int64_t>() : 0);
                    vm.try_stack.push_back(catch_ip);
                }
                vm.ip++;
                break;
            }

            case Opcode::OpTryEnd: {
                if (!vm.try_stack.empty()) {
                    vm.try_stack.pop_back();
                }
                vm.ip++;
                break;
            }

            case Opcode::OpThrow: {
                if (!vm.try_stack.empty()) {
                    std::size_t catch_ip = vm.try_stack.back();
                    vm.try_stack.pop_back();
                    vm.ip = catch_ip; // Direct jump to exception handler
                } else {
                    vm.halted = true;
                    return Status::Done;
                }
                break;
            }

            default:
                vm.ip++;
                break;
            }
        }

        if (vm.halted || vm.ip >= code_.size()) return Status::Done;
        return Status::Continue; // Preempted when reductions == 0
    }

private:
    std::int64_t get_int_val(const Term& t) const {
        if (auto* i = t.get_if<std::int64_t>()) return *i;
        if (auto* a = t.get_if<Atom>()) return (a->name == "true" || a->name == "benar") ? 1LL : 0LL;
        return 0LL;
    }

    Term read_operand(const GvmProcessState& vm, const Operand& op) const {
        if (op.type == RegType::X) return vm.x_regs[op.index];
        if (op.type == RegType::Y && op.index < vm.y_stack.size()) return vm.y_stack[op.index];
        return op.literal;
    }

    void write_operand(GvmProcessState& vm, const Operand& op, Term val) const {
        if (op.type == RegType::X) vm.x_regs[op.index] = std::move(val);
        else if (op.type == RegType::Y) {
            if (op.index >= vm.y_stack.size()) vm.y_stack.resize(op.index + 1);
            vm.y_stack[op.index] = std::move(val);
        }
    }

    std::vector<Instruction> code_;
    SpawnIpCallback spawn_cb_;
    SendMsgCallback send_cb_;
    ReceiveMsgCallback recv_cb_;
};

} // namespace garuda::beam
