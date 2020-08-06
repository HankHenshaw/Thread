#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include "observer.h"

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    int size;
    if(argc < 2)
    {
        size = 1;
    } else {
        if(atoi(argv[1]) < 1) {
            size = 1;
        } else {
            size = atoi(argv[1]);
        }
    }

    Subject subj(size);

    subj.AddSub(std::make_shared<FileObserver>(subj));
    subj.AddSub(std::make_shared<FileObserver>(subj));
    subj.AddSub(std::make_shared<CoutObserver>(subj));

    char ch;
    while(std::cin >> ch)
    {
        std::this_thread::sleep_for(1s);
        subj.AddCmd(ch);
    }
    std::this_thread::sleep_for(1s);
    subj.AddCmd();

    return 0;
}
