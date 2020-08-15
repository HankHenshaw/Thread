#include "observer.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <string>

std::condition_variable cv;
std::atomic_bool quit = false;
std::mutex cv_m;
std::mutex out_lock;

std::atomic_bool isCoutGetQueue = false;
std::atomic_bool isName = false; // TODO: Rename

std::atomic_int fileThreadId = 0;

int Subject::fileSubscriber = 0;
std::unordered_map<std::string, Metrics> Subject::m_metrics;


void Subject::AddSub(std::shared_ptr<IObserver> &&sub)
{
    auto obj = sub.get();
    std::thread th1(&IObserver::printRestQueue, obj, std::ref(m_queue), Subject::fileSubscriber);

//    if(typeid(*sub).name() == typeid(FileObserver).name())
//    {
//        std::cout << "File\n";
//        ++Subject::fileSubscriber;
//    }

    ++Subject::fileSubscriber;

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

    if(m_stack.empty() && ch == '{')
    {
        isNestedBlock = true; // Вложенный блок
        m_stack.push(ch);
        return;
    } else if(ch == '}') {
        if(isNestedBlock)
        {
            isNestedBlock = false;
            m_stack.pop();
            ++m_main.m_blocks;
            cv.notify_all();
        }
        return;
    } else if(ch == '{') { // Блок внутри блока не обрабатываем
        return;
    }

    m_queue.push(ch);
    isName = true;
    ++m_counter;
    if(m_counter == m_blockSize)
    {
        m_counter = 0;
        cv.notify_all();
    }

    ++m_main.m_commands; // Увеличиваем комманды
}


size_t Subject::SizeOfSubs() const
{
    return m_listOfSubs2.size();
}

void Subject::printMetrics()
{
    std::cout << "Size: " << Subject::m_metrics.size() << '\n';
    for(const auto& val: Subject::m_metrics)
    {
        std::cout << val.first << '\n'
                  << "-    Commands:" << val.second.m_commands << '\n'
                  << "-    Blocks:" << val.second.m_blocks << '\n';
        if(val.second.m_lines)
        {
            std::cout << "-    Lines:" << val.second.m_lines << '\n';
        }
        std::cout << "\n------------------\n";
    }
}

Subject::~Subject()
{
    quit = true;
    cv.notify_all();
    Subject::m_metrics.emplace(std::make_pair("Main Thread:", m_main));
    for(auto&& val: m_vecOfThreads)
    {
        val.join();
    }

    printMetrics();
}

FileObserver::~FileObserver()
{
    std::cout << "File Thread N" << m_id+1 << '\n'
              << "-    Commands:" << m_metric.m_commands << '\n'
              << "-    Blocks:" << m_metric.m_blocks << '\n';
    std::cout << "\n-------------------\n";
    Subject::m_metrics.emplace(std::make_pair("File Thread N"+std::to_string(m_id), m_metric));
}

long FileObserver::printTime() const
{
    auto tm = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(tm.time_since_epoch()).count();
    return time;
}

void FileObserver::printRestQueue(std::queue<char> &queue, int id)
{
    m_id = id;

    std::string filename("bulk" + std::to_string(printTime()) + "_" + std::to_string(Subject::fileSubscriber) + ".log");
    std::ofstream file(filename, std::ios_base::out);

    while(!quit)
    {
        isName = false;
        std::unique_lock<std::mutex> lk(cv_m); // Что бы поток засыпал
        cv.wait(lk, [&queue, &id](){return (!queue.empty() || quit) && (isCoutGetQueue || quit) && (fileThreadId%2 == id || quit);});
        std::cout << "Fibo woke up\n";

        if(!queue.empty() && isCoutGetQueue && fileThreadId%2 == id) // Возможно тут нужен while т.к. в последствии
        {//тут будут не одиночные комманды, а блоки
            ++fileThreadId;
            isCoutGetQueue = false; // ???

            while(!queue.empty())
            {
                ++m_metric.m_commands;

                auto val = queue.front();
                queue.pop();
                lk.unlock(); // TODO: Comment this?

//                out_lock.lock();
//                file << fibo(val - 48) << '\n';
                file << val << '\n';
//                out_lock.unlock();
            }
        }
    }

    file.close();
}

CoutObserver::~CoutObserver()
{
    std::cout << "Cout Thread" << '\n'
              << "-    Commands:" << m_metric.m_commands << '\n'
              << "-    Blocks:" << m_metric.m_blocks << '\n';
    std::cout << "\n-------------------\n";
    Subject::m_metrics.emplace(std::make_pair("Log Thread:", m_metric));
}

void CoutObserver::printRestQueue(std::queue<char> &queue, int id)
{
    m_id = id;

    std::queue<char> tmp_queue; //Копия очереди, что бы вывести значения
    while(!quit) // Что бы поток был внутри этого цикла пока мы не завершим ввод
    {
        std::unique_lock<std::mutex> lk(cv_m); // Что бы поток засыпал
        cv.wait(lk, [](){return isName || quit;}); //Пока в основной очереди не появятся эл-ты или сигнал о выходе

        std::cout << "Fact woke up\n";

        if(!queue.empty())
        {
            tmp_queue = queue; // Если проснулись то делаем копию очереди

            isCoutGetQueue = true; //???
            cv.notify_all(); //Оповещаем остальные потоки что можно работать с очередью

            int size = tmp_queue.size();
            int i = 0;
            while (!tmp_queue.empty()) // Простой цикл для вывода всех значений очереди
            {
                ++m_metric.m_commands;
                ++i;
                if(i == size)
                {
                    isName = false;
                }

                auto val = tmp_queue.front();
                tmp_queue.pop();

                out_lock.lock();
                //std::cout << fact(val - 48) << '\n';
                std::cout << val << '\n';
                out_lock.unlock();
            }
        } else {
            isName = false;
        }
    }
//    out_lock.lock();
//    std::cout << "Fact ended\n";
//    out_lock.unlock();
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