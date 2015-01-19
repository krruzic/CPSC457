#include <stdlib.h>
#include <sys/ptrace.h> // ptrace calls
#include <sys/types.h>
#include <unistd.h>
#include <udis86.h>  // library for disassembly   
#include <sys/user.h> // for user_regs_struct
#include <stdio.h>
#include <sys/wait.h> 
#include <string.h> // memcpy

int disassemble(int traced_process, unsigned char *data);


int main(int argc, char *argv[]) {
    pid_t traced_process;
    int wait_val;
    unsigned int instruct[4];
    struct user_regs_struct regs;
    unsigned char *data;
    if (argc != 3) {
        printf("Usage: %s -p <pid to be traced>\n", argv[0], argv[1]);
        return -1;
    }
    traced_process = atoi(argv[2]); // converts input to an int
    ptrace(PTRACE_ATTACH, traced_process, NULL, NULL); // attaches to process specified
    wait(&wait_val);

    while(WIFSTOPPED(wait_val)) { // loops while the program is stopped
        ptrace(PTRACE_GETREGS, traced_process, NULL, &regs);
        int i;
        for (i = 0; i < 4; i++)
            instruct[i] = ptrace(PTRACE_PEEKTEXT, traced_process, regs.eip+(i*4), NULL);
        memcpy(data, instruct, 15); // copy our 15 bits into one buffer


        printf("DATA:\t");
        disassemble(traced_process, data); // disassemble instruction
        ptrace(PTRACE_SINGLESTEP, traced_process, 0, 0); // move to the next instruction
        wait(&wait_val); // wait for singlestep to finish
    }

    ptrace(PTRACE_DETACH, traced_process, NULL, NULL); // detaches and kills attached process
    return 0;
}

/* Mostly just set up code. Disassembles the instruction in the last line */
int disassemble(int traced_process, unsigned char *data) {    
    ud_t ud_obj;
    ud_init(&ud_obj);     
    ud_set_mode(&ud_obj, 32);
    ud_set_input_buffer(&ud_obj, data, 32);
    ud_set_syntax(&ud_obj, UD_SYN_INTEL);
    ud_disassemble(&ud_obj);
    printf("%s\n", ud_insn_asm(&ud_obj));
    return 0;

}

