#include <stdint.h>
#include <vector>
#include <stack>

using namespace std;

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

struct state_t {
	vector<uint64_t> dmem;
	vector<uint64_t> reg;
	stack<void*>     ret_stack;
	uint64_t         carry;
	state_t():
		dmem(1024),
		reg(32),
		ret_stack(),
		carry() {}
};

extern state_t *state;

#define BRZ(src, label) if(state->reg.at(src)==0) {goto label;}

#define BRNZ(src, label) if(state->reg.at(src)!=0) {goto label;}

#define CALL(dest) \
	state->ret_stack.push(&& TOKENPASTE2(Unique_, __LINE__) ); goto dest; TOKENPASTE2(Unique_, __LINE__) :

#define RET \
	goto state->ret_stack.pop();

inline void ADD(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) + state->reg.at(src2);
}

inline void SUB(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) - state->reg.at(src2);
}

inline void ADDI(int dst, int src1, uint64_t src2) {
	state->reg.at(dst) = state->reg.at(src1) + src2;
}

inline void SUBI(int dst, int src1, uint64_t src2) {
	state->reg.at(dst) = state->reg.at(src1) - src2;
}

inline void XOR(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) ^ state->reg.at(src2);
}

inline void OR(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) | state->reg.at(src2);
}

inline void AND(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) & state->reg.at(src2);
}

inline void NOT(int dst, int src) {
	state->reg.at(dst) = ~ state->reg.at(src);
}

inline void SHL(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) << state->reg.at(src2);
}

inline void SHR(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) >> state->reg.at(src2);
}

inline void SHLI(int dst, int src1, uint64_t src2) {
	state->reg.at(dst) = state->reg.at(src1) << src2;
}

inline void SHRI(int dst, int src1, uint64_t src2) {
	state->reg.at(dst) = state->reg.at(src1) >> src2;
}

inline void SLT(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) < state->reg.at(src2);
}

inline void SLE(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) <= state->reg.at(src2);
}

inline void SEQ(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) == state->reg.at(src2);
}

inline void SNE(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) != state->reg.at(src2);
}

inline void SGT(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) > state->reg.at(src2);
}

inline void SGE(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) >= state->reg.at(src2);
}

inline void LD(int dst, int base, uint64_t offset) {
	state->reg.at(dst) = state->dmem.at(state->reg.at(base) + offset);
}

inline void ST(int src, int base, uint64_t offset) {
	state->dmem.at(state->reg.at(base) + offset) = state->reg.at(src);
}

inline void ADC(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) + state->reg.at(src2) + state->carry;
	if(state->reg.at(dst) < state->reg.at(src1)) {
		state->carry = 1;
	} else if(state->reg.at(dst) < state->reg.at(src2)) {
		state->carry = 1;
	} else {
		state->carry = 0;
	}
}

inline void SBB(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) - state->reg.at(src2) - state->carry;
	if(state->reg.at(src1) < state->reg.at(src2) + state->carry) {
		state->carry = 1;
	} else {
		state->carry = 0;
	}
}

inline void MULLO(int dst, int src1, int src2) {
	state->reg.at(dst) = state->reg.at(src1) * state->reg.at(src2);
}

inline void MUL(int dst, int src1, int src2) {
	unsigned __int128 d, s1, s2;
	s1 = (unsigned __int128)state->reg.at(src1);
	s2 = (unsigned __int128)state->reg.at(src2);
	d = s1 * s2;
	state->reg.at(dst) = (uint64_t)d;
	state->reg.at(dst+1) = (uint64_t)(d>>64);
}

inline void MAC(int dst, int src1, int src2) {
	unsigned __int128 d, s1, s2;
	s1 = (unsigned __int128)state->reg.at(src1);
	s2 = (unsigned __int128)state->reg.at(src2);
	d = (unsigned __int128)state->reg.at(dst+1);
	d = (d<<64)|(unsigned __int128)state->reg.at(dst);
	d += s1 * s2;
	state->reg.at(dst) = (uint64_t)d;
	state->reg.at(dst+1) = (uint64_t)(d>>64);
}

inline void MACC(int dst, int src1, int src2) {
	unsigned __int128 d, s1, s2, prod, orig_d;
	s1 = (unsigned __int128)state->reg.at(src1);
	s2 = (unsigned __int128)state->reg.at(src2);
	d = (unsigned __int128)state->reg.at(dst+1);
	d = (d<<64)|(unsigned __int128)state->reg.at(dst);
	prod = s1 * s2 ;
	orig_d = d;
	d = d + prod + (unsigned __int128)state->carry;
	state->reg.at(dst) = (uint64_t)d;
	state->reg.at(dst+1) = (uint64_t)(d>>64);
	state->carry = (d < orig_d || d < prod);
}

