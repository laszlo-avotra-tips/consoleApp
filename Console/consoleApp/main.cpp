/*
 * main.cpp
 *
 * The OCT Console application
 *
 * Author: Chris White, Dennis W. Jackson, Ryan Radjabi
 *
 * Copyright (c) 2009-2018 Avinger, Inc.
 *
 */
#include "AxsunOCTCapture.h"        // for all AxsunOCTCapture functions, structs, enums, typedefs
#include <iostream>                 // for std::cout, std::cin; not related to AxsunOCTCapture functionality
#include <chrono>                   // for loop timer sleep function; not related to AxsunOCTCapture functionality
#include <thread>                   // for loop timer sleep function; not related to AxsunOCTCapture functionality
#include <conio.h>                  // for console keyboard interaction; not related to AxsunOCTCapture functionality
#include <windows.h>                // for Win32 event handling

using namespace std::chrono_literals;

//#define PCIEMODE                    // comment this line out for Ethernet interface, leave uncommented for PCIe interface

class AxsunOCTCapture {
private:
    AOChandle session_{ nullptr };
public:
    AxsunOCTCapture(int32_t capacity) { if (auto retval = axStartSession(&session_, capacity); retval != AxErr::NO_AxERROR) throw retval; }  // C++17
    ~AxsunOCTCapture() { axStopSession(session_); }
    AOChandle operator()() { if (session_) return session_; else throw AxErr::CAPTURE_SESSION_NOT_SETUP; }
};

/*
 * main
 */
int main(int argc, char *argv[])
{
    char message[512];      // variable for getting string output from axGetMessage() and axGetErrorString()

    try {
        std::cout << "Welcome to the AxsunOCTCapture Example Console Application (C++ version).\n";

        // create a capture session with 500 MB main image buffer
        AxsunOCTCapture AOC(500);

        // select capture interface for this session
#ifdef PCIEMODE
        if (auto retval = axSelectInterface(AOC(), AxInterface::PCI_EXPRESS); retval != AxErr::NO_AxERROR) throw retval;
#else
        if (auto retval = axSelectInterface(AOC(), AxInterface::GIGABIT_ETHERNET); retval != AxErr::NO_AxERROR) throw retval;
#endif

        // print message describing current capture interface status
        if (auto retval = axGetMessage(AOC(), message); retval != AxErr::NO_AxERROR) throw retval;
        std::cout << message << '\n';

    }
    catch (const AxErr& e) {
        axGetErrorString(e, message);
        std::cout << "ERROR: " << message << '\n';
    }
    catch (...) {
        std::cout << "***** UNKNOWN ERROR. Program terminating.\n";
    }

    std::cout << "Quitting...\n\n";

}
