/**
 * @file observer.cpp
 * @author HankHenshaw (you@domain.com)
 * @brief Реализация методов для классов субъекта и наблюдателей
 * @version 0.1
 * @date 2020-07-09
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "observer.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <string>

//1 - Консольный поток не должен изменять состояние очереди?
//2 - При таком строении методов, консольный поток должен вызываться 1-ым - тут-то проблема и кроется
//3 - Если нотофицировать 1 поток когда объект является консольным выводом, то консольный поток может не прбудиться
//4 - Если нотофицировать все - то п.2

std::condition_variable cv;
std::atomic_bool quit = false;
std::mutex cv_m;
std::mutex out_lock;

int Subject::fileSubscriber = 0;

void Subject::Notify()
{
//    std::for_each(m_listOfSubs2.begin(), m_listOfSubs2.end(), [this](const auto& sub){(*sub).Update(this->m_queue);});

    bool isFileUpdate = false;
    for(const auto& val: m_listOfSubs2)
    {
        if(typeid(*val).name() == typeid(FileObserver).name())
        {
            if(m_counterFile == m_currentNumber)
            {
                cv.notify_one();
                isFileUpdate = true;
            }
            ++m_currentNumber;
        } else {
//            (*val).Update(this->m_queue); //TODO: Remove?
            (*val).printRestQueue(this->m_queue); //Сделать отдельным потоком
//            cv.notify_all();
        }
    }

    if(isFileUpdate) ++m_counterFile;
    m_counterFile %= Subject::fileSubscriber;
    m_currentNumber = 0;
}

void Subject::AddSub(std::shared_ptr<IObserver> &&sub)
{
    if(typeid(*sub).name() == typeid(FileObserver).name())
    {
        std::cout << "File\n";
        ++Subject::fileSubscriber;
    }
    auto obj = sub.get();
    std::thread th1(&IObserver::printRestQueue, obj, std::ref(m_queue));
    m_vecOfThreads.emplace_back(std::move(th1));
    m_listOfSubs2.emplace_back(std::move(sub));
}

void Subject::RemSub(std::shared_ptr<IObserver> &&sub)
{
    auto it = std::find(m_listOfSubs2.begin(), m_listOfSubs2.end(), sub);
    if(it == m_listOfSubs2.end())
    {
        throw std::logic_error("Subscriber not found\n");
    } else {
        m_listOfSubs2.erase(it);
    }
}

void Subject::AddCmd(char ch)
{
    ++m_main.m_lines; // Увеличиваем строку

    // Временно не используется
    // if(m_stack.empty() && ch == '{')
    // {
    //     isNestedBlock = true;
    //     m_counter = 0;
    //     m_stack.push(ch);
    //     Notify();
    //     return;
    // } else if(ch == '{') {
    //     m_stack.push(ch);
    //     isNestedBlock = true;
    //     m_counter = 0;
    //     return;
    // } else if(ch == '}') {
    //     isNestedBlock = false;
    //     if(!m_stack.empty()) {
    //         m_stack.pop();
    //         if(m_stack.empty()) {
    //             ++m_main.m_blocks; // Увеличиваем кол-во блоков
    //             Notify();
    //         }
    //     }
    //     return;
    // }

    ++m_main.m_commands; // Увеличиваем комманды
    m_queue.push(ch);

    if(!isNestedBlock)
        ++m_counter;
//    std::cout << "Counter = " << m_counter << '\n';
    if((m_counter%m_blockSize == 0) && (m_stack.empty()))
    {
        m_counter = 0;
        Notify();
    }
}

void Subject::AddCmd()
{
    if(!m_queue.empty() && m_stack.empty())
    {
        Notify();
    }
}

size_t Subject::SizeOfSubs() const
{
    return m_listOfSubs2.size();
}

Subject::~Subject()
{
    quit = true;
    cv.notify_all();
    for(auto&& val: m_vecOfThreads)
    {
        val.join();
    }
}

long FileObserver::printTime() const
{
    auto tm = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(tm.time_since_epoch()).count();
    return time;
}

void FileObserver::Update(std::queue<char> queue)
{
    printRestQueue(queue);
}

//void FileObserver::printRestQueue(std::queue<char> &queue)
//{
//    if(queue.empty()) return;

//    std::string filename("bulk" + std::to_string(printTime()) + "_" + std::to_string(m_number) + ".log");
//    std::ofstream file(filename, std::ios_base::out);

//    if(!file.is_open())
//    {
//        throw std::runtime_error("log file won't open");
//    }

//    while(!queue.empty())
//    {
//        file << queue.front();
//        queue.pop();
//    }
//    file << '\n';

//    file.close();
//}

void FileObserver::printRestQueue(std::queue<char> &queue)
{
    std::string filename("bulk" + std::to_string(printTime()) + "_" + std::to_string(Subject::fileSubscriber) + ".log");
    std::ofstream file(filename, std::ios_base::out);

    while(!quit)
    {
        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, [&queue](){return !queue.empty() || quit;});
        if(!queue.empty())
        {
            int val = queue.front();
            queue.pop();
            lk.unlock();

            out_lock.lock();
            file << fibo(val - 48) << '\n';
            out_lock.unlock();
        }
    }

    file.close();
}

void CoutObserver::Update(std::queue<char> queue)
{
    printRestQueue(queue);
}

void CoutObserver::printRestQueue(std::queue<char> &queue)
{
    if(!queue.empty())
    {
        int val = queue.front() - 48;
        out_lock.lock();
        std::cout << fact(val) << '\n';
        out_lock.unlock();
    }
}

size_t fibo(size_t val)
{
    if(!val) return 0;
    if(val == 1) return 1;
    if(val > 44) throw std::invalid_argument("val > 44 can't be stored in unsigned int");
    return fibo(val - 1) + fibo(val - 2);
}

size_t fact(size_t val)
{
    if(!val || val == 1) return 1;
    if(val > 12) throw std::invalid_argument("val > 12 can't be stored in unsigned int");
    return val*fact(val - 1);
}