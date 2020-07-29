/**
 * @file observer.h
 * @author HankHenshaw (you@domain.com)
 * @brief Определения классов наблюдателя и субъекта
 * @version 0.1
 * @date 2020-07-09
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#pragma once

#include <list>
#include <stack>
#include <queue>
#include <memory>

/**
 * @brief Общий интерфейс для всех наблюдателей
 * 
 */
class IObserver {
public:
    virtual ~IObserver() {};
    virtual void Update(std::queue<char> queue) = 0;
};
/**
 * @brief Класс Субъекта, который оповещает своих наблюдателей
 * 
 */
class Subject {
    std::list<std::unique_ptr<IObserver>> m_listOfSubs2;
    std::list<IObserver*> m_listOfSubs;
    std::stack<char> m_stack;
    std::queue<char> m_queue;
    int m_counter;
    int m_blockSize;
    bool isNestedBlock;
public:
    Subject(int blockSize) : m_counter(0), m_blockSize(blockSize), isNestedBlock(false) {};
    ~Subject() {};

    void AddCmd(char ch);
    void AddCmd();
    void AddSub(std::unique_ptr<IObserver> &&sub);
    void RemSub(std::unique_ptr<IObserver> &&sub);
    void Notify();
    size_t SizeOfSubs() const;
};
/**
 * @brief Классы вывода команд в файл
 * 
 * название файла будет таким: bulk*.log, где * - время в формате unixtime
 */
class FileObserver : public IObserver {
    Subject &m_subject;
public:
    FileObserver(Subject &sub) : m_subject(sub) {}

    virtual void Update(std::queue<char> queue) override;
    virtual ~FileObserver() {};

    long printTime() const;
    void printRestQueue(std::queue<char> &queue);
};

/**
 * @brief Класс вывода в стандартный поток
 * 
 */
class CoutObserver : public IObserver {
    Subject &m_subject;
public:
    CoutObserver(Subject &sub) : m_subject(sub) {}

    virtual void Update(std::queue<char> queue) override;
    virtual ~CoutObserver() {};

    void printRestQueue(std::queue<char> &queue);
};