#pragma once

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

struct Metrics {
    int m_lines;
    int m_commands;
    int m_blocks;

    Metrics() : m_lines(0), m_commands(0), m_blocks(0) {};
};

class IObserver {
public:
    virtual ~IObserver() {};
    virtual void Update(std::queue<char> queue) = 0;
    virtual void printRestQueue(std::queue<char> &queue) = 0;
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

public:
    Subject(int blockSize) : m_counter(0), m_blockSize(blockSize), isNestedBlock(false), m_currentNumber(0), m_counterFile(0) {};
    ~Subject();

    void AddCmd(char ch);
    void AddCmd();
    void AddSub(std::shared_ptr<IObserver> &&sub);
    void RemSub(std::shared_ptr<IObserver> &&sub);
    void Notify();
    size_t SizeOfSubs() const;

    Metrics m_main; //TODO Make private

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
    virtual void printRestQueue(std::queue<char> &queue) override;
};


class CoutObserver : public IObserver {
    Subject &m_subject;
public:
    CoutObserver(Subject &sub) : m_subject(sub) {}

    virtual void Update(std::queue<char> queue) override;
    virtual ~CoutObserver() {};

    virtual void printRestQueue(std::queue<char> &queue) override;
};

size_t fibo(size_t val);
size_t fact(size_t val);