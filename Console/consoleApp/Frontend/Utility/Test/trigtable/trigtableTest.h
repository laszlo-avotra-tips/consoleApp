/*
 * trigTableTest.h
 *
 * Unit test for trig lookup table object.
 */

#include <qttest/qttest>

class trigTableTest: public QObject
{
  Q_OBJECT

  private slots:
  void lookupSin();
  void lookupCos();
  void lookupPos();

};
