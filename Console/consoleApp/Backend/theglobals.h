#ifndef THEGLOBALS_H
#define THEGLOBALS_H

#include "octFile.h"
#include "defaults.h"
#include "AlazarApiType.h"
#include <QMutex>
#include <vector>


class TheGlobals
{
public:
    static TheGlobals* instance();

    int getGDaqRawData_idx() const;
    int getPrevGDaqRawData_idx() const;
    void updateGDaqRawData_idx();

    int getGDaqRawDataBuffersPosted() const;

    int getGDaqRawDataCompleted() const;
    void incrementGDaqRawDataCompleted();

    int getGFrameCounter() const;
    int getPrevGFrameCounter() const;
    void inrementGFrameCounter();

    OCTFile::FrameData_t * getFrameDataPointer(int index) const;
    OCTFile::FrameData_t * getFrameDataPointer() const;
    void freeFrameData();
    void allocateFrameData();

    const U16* getDaqRawDataBuffer(size_t index) const;
    const U16* getDaqRawDataBuffer() const;
    void freeDaqRawDataBuffer();
    void allocateDaqRawDataBuffer(U32 bufferSize);
    void allocateDaqRawDataBuffer();

private:
    static TheGlobals* m_instance;
    TheGlobals();

private:
    // Create a block of memory for storing the data streaming
    // from the DAQ hardware. The array is indexed by m_gFrameCounter to point
    // to the current FrameDate_t to write to.  The memory is allocated all
    // at once to avoid calling on the OS to create and release memory during
    // run-time.
    std::vector<OCTFile::FrameData_t> m_gFrameData;
    std::vector<U16*> m_daqRawData;
    mutable QMutex m_mutex;
    const int m_daqRawDataBufferCount;
    int m_gFrameCounter;
    int m_gDaqRawData_idx;
    int m_gDaqRawDataBuffersPosted;
    int m_gDaqRawDataCompleted;
};

#endif // THEGLOBALS_H
