#ifndef THEGLOBALS_H
#define THEGLOBALS_H

#include "octFile.h"
#include "defaults.h"
#include <QMutex>
#include <vector>
#include <queue>

using OctData = OCTFile::OctData_t;

class TheGlobals
{
public:
    static TheGlobals* instance();

    OCTFile::OctData_t * getOctData(int index);
    void freeOctData();
    void allocateOctData();

    void pushImageRenderingQueue(OctData od);
    void popImageRenderingQueue();
    bool isImageRenderingQueueGTE(size_t length) const;
    std::pair<bool, OctData>  frontImageRenderingQueue();

private:
    static TheGlobals* m_instance;
    TheGlobals();

private:
    std::vector<OctData> m_octData;
    QMutex m_imageRenderingMutex;
    std::queue<OctData> m_imageRenderingQueue;
};

#endif // THEGLOBALS_H
