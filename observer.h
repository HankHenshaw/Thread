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
    virtual void printRestQueue(std::queue<char> &queue, int id) = 0;
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

    void printMetrics();
public:
    Subject(int blockSize) : m_counter(0), m_blockSize(blockSize), isNestedBlock(false), m_currentNumber(0), m_counterFile(0) {};
    ~Subject();

    void AddCmd(char ch);
    void AddSub(std::shared_ptr<IObserver> &&sub);
    void RemSub(std::shared_ptr<IObserver> &&sub);
    size_t SizeOfSubs() const;

    Metrics m_main; //TODO Make private
    static std::unordered_map<std::string, Metrics> m_metrics;

    static int fileSubscriber;
};


class FileObserver : public IObserver {
    Metrics m_metric;
    Subject &m_subject;
    int m_number;
    int m_id;
public:
    FileObserver(Subject &sub) : m_subject(sub), m_number(Subject::fileSubscriber + 1) {}

    virtual ~FileObserver();

    long printTime() const;
    virtual void printRestQueue(std::queue<char> &queue, int id) override;
};


class CoutObserver : public IObserver {
    Metrics m_metric;
    Subject &m_subject;
    int m_id;
public:
    CoutObserver(Subject &sub) : m_subject(sub) {}

    virtual ~CoutObserver();

    virtual void printRestQueue(std::queue<char> &queue, int id) override;
};

size_t fibo(size_t val);
size_t fact(size_t val);