#pragma once

#include <iostream>
#include <list>
#include <stack>
#include <queue>
#include <memory>
#include <typeinfo>
#include <thread>
#include <condition_variable>
#include <future>
#include <vector>
#include <functional>

class IObserver {
public:
    virtual ~IObserver() {};
    virtual void Update([[maybe_unused]]std::queue<char> queue) {};
};

class Subject {  
    std::list<std::shared_ptr<IObserver>> m_listOfSubs2;
    std::list<IObserver*> m_listOfSubs;
    std::stack<char> m_stack;
    std::queue<char> m_queue;
    int m_counter;
    int m_blockSize;
    bool isNestedBlock;
    int m_currentNumber;
    int m_counterFile;
    std::vector<std::thread> m_vecOfThreads;
    bool m_isThreadInit;
public:
    Subject(int blockSize) : m_counter(0), m_blockSize(blockSize), isNestedBlock(false), m_currentNumber(0), m_counterFile(0),
                             m_isThreadInit(false) {};
    ~Subject();

    void AddCmd(char ch);
    void AddCmd();
    void AddSub(std::shared_ptr<IObserver> &&sub);
    void RemSub(std::shared_ptr<IObserver> &&sub);
    void Notify();
    size_t SizeOfSubs() const;

    static int fileSubscriber;
};


class FileObserver : public IObserver {
    Subject &m_subject;
    int m_number;
public:
    FileObserver(Subject &sub) : m_subject(sub), m_number(Subject::fileSubscriber + 1) {}

    virtual void Update(std::queue<char> queue) override;
    virtual ~FileObserver() {};

    long printTime() const;
    void printRestQueue(std::queue<char> &queue);
};


class CoutObserver : public IObserver {
    Subject &m_subject;
public:
    CoutObserver(Subject &sub) : m_subject(sub) {}

    virtual void Update(std::queue<char> queue) override;
    virtual ~CoutObserver() {};

    void printRestQueue(std::queue<char> &queue);
};
