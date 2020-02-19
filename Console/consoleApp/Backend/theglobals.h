#ifndef THEGLOBALS_H
#define THEGLOBALS_H

#include "octFile.h"
#include "defaults.h"
#include <QMutex>
#include <vector>
#include <queue>


class TheGlobals
{
public:
    static TheGlobals* instance();

    OCTFile::OctData_t * getOctData(int index);
    void freeOctData();
    void allocateOctData();

    void pushImageRenderingQueue(int index);
    void popImageRenderingQueue();
    bool isImageRenderingQueue() const;
    int  frontImageRenderingQueue() const;

private:
    static TheGlobals* m_instance;
    TheGlobals();

private:
    std::vector<OCTFile::OctData_t> m_octData;
    QMutex m_imageRenderingMutex;
    std::queue<int> m_imageRenderingQueue;
};

#endif // THEGLOBALS_H
