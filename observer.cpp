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
#include <typeinfo>

void Subject::Notify()
{
    for(const auto &val : m_listOfSubs2)
    {
        if(typeid(*val).name() == typeid(FileObserver).name())
        {
            std::cout << "File\n";
        } 
        // else {
        //     std::cout << "Cout\n";
        // }
    }

    std::for_each(m_listOfSubs2.begin(), m_listOfSubs2.end(), [this](const auto& sub){(*sub).Update(this->m_queue);});
    while(!m_queue.empty()) 
        m_queue.pop();
}

void Subject::AddSub(std::unique_ptr<IObserver> &&sub)
{
    m_listOfSubs2.emplace_back(std::move(sub));
}

void Subject::RemSub(std::unique_ptr<IObserver> &&sub)
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
    if(m_stack.empty() && ch == '{')
    {
        isNestedBlock = true;
        m_counter = 0;
        m_stack.push(ch);
        Notify();
        return;
    } else if(ch == '{') {
        m_stack.push(ch);
        isNestedBlock = true;
        m_counter = 0;
        return;
    } else if(ch == '}') {
        isNestedBlock = false;
        if(!m_stack.empty()) {
            m_stack.pop();
            if(m_stack.empty()) {
                Notify();
            }
        }
        return;
    }
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

void FileObserver::printRestQueue(std::queue<char> &queue)
{
    if(queue.empty()) return;

    std::string filename("bulk" + std::to_string(printTime()) + ".log");
    std::ofstream file(filename, std::ios_base::out);

    if(!file.is_open())
    {
        throw std::runtime_error("log file won't open");
    }

    while(!queue.empty())
    {
        file << queue.front();
        queue.pop();
    }
    file << '\n';

    file.close();
}


void CoutObserver::Update(std::queue<char> queue)
{
    printRestQueue(queue);
}

void CoutObserver::printRestQueue(std::queue<char> &queue)
{
    if(queue.empty()) return;

    while(!queue.empty())
    {
        std::cout << queue.front();
        queue.pop();
    }
    std::cout << '\n';
}
