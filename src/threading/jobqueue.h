// ============================================================================
// jobqueue.h - job queue batch descriptor (56 bytes, verified against IDA).
// Source: jobqueue_xboxr:jobqueue.o (jobqueue.h)
// ============================================================================
#ifndef COD3_THREADING_JOBQUEUE_H
#define COD3_THREADING_JOBQUEUE_H

struct jqModule;

struct jqBatch {
    void*        Input;       // jqPtr<void>   +0x00
    void*        Output;      // jqPtr<void>   +0x04
    void*        Scratch;     // jqPtr<void>   +0x08
    void*        Static;      // jqPtr<void>   +0x0C
    unsigned int InputSize;   // +0x10
    unsigned int OutputSize;  // +0x14
    unsigned int ScratchSize; // +0x18
    unsigned int StaticSize;  // +0x1C
    int          Handle;      // +0x20
    jqModule*    Module;      // +0x24
    int          Priority;    // +0x28
    int          GroupID;     // +0x2C
    int          Next;        // +0x30
    unsigned char _Batch;     // +0x34 (_jqBatch)
};
static_assert(sizeof(jqBatch) == 0x38, "jqBatch size mismatch");

#endif // COD3_THREADING_JOBQUEUE_H
