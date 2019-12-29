INCLUDEPATH += ../../../Include
SOURCES = test.cpp ../../Integrator.cpp
CONFIG  += qtestlib

# install
# target.path = $$[QT_INSTALL_EXAMPLES]/qtestlib/tutorial1
sources.files = $$SOURCES *.pro
sources.path = .
INSTALLS += sources #target
