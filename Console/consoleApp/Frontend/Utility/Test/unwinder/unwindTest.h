/*
 * unwindTest.h
 *
 * Unit test for the lag correction object.
 *
 * Author: Chris White
 *
 * Copyright 2009 Sawtooth Labs, Inc.
 */ 

#include <qttest/qttest>

class unwindTest: public QObject
{
  Q_OBJECT

  private slots:
  void testUnwind();
};
