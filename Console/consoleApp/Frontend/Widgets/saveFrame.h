#ifndef SAVEFRAME_H
#define SAVEFRAME_H

#include <cstdint>
#include <QString>

class QFile;
class SaveFrame
{
public:
    static SaveFrame* instance();
    void saveBuffer(const uint8_t* buffer, int count);

    quint64 bytesWritten() const;

private:
    static SaveFrame* m_instance;
    const int m_countBegin = 200;
    const int m_countEnd = 1200;
    QString m_fn{(R"(c:/Avinger_System/recordedimages.dat)")};
    QFile* m_file;
    quint64 m_bytesWritten{0};

private:
    SaveFrame();
};

#endif // SAVEFRAME_H
