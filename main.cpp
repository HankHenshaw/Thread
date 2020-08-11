#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <queue>
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

    subj.AddSub(std::make_shared<CoutObserver>(subj)); //TODO: Проверить можно ли использовать unique_ptr
    subj.AddSub(std::make_shared<FileObserver>(subj));
    subj.AddSub(std::make_shared<FileObserver>(subj));

    char ch;
    while(1)
    {
        std::cin >> ch;
        if(ch == '!') break;
        std::this_thread::sleep_for(1s);
        subj.AddCmd(ch);
    }
    std::cout << "Lines = " << subj.m_main.m_lines << '\n';
    std::cout << "Commands = " << subj.m_main.m_commands << '\n';
    std::cout << "Blocks = " << subj.m_main.m_blocks << '\n';
    std::cout << "Done!\n";
//    std::this_thread::sleep_for(1s);
//    subj.AddCmd();
    return 0;
}