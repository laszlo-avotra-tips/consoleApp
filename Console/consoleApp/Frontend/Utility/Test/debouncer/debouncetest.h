/*
 * debounceTest.cpp
 *
 * Unit test for the debounce object.
 */

#include <qttest/qttest>

class debounceTest: public QObject
{
  Q_OBJECT

    private slots:
  void testAngleDebounce();
  void testIntDebounce();

};
